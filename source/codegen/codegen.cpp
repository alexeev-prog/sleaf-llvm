#include "codegen/codegen.hpp"

#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include "logger.hpp"

namespace sleaf {

    CodeGenerator::CodeGenerator()
        : module(std::make_unique<llvm::Module>("main", context))
        , builder(context) {}

    void CodeGenerator::generate(const std::vector<std::unique_ptr<Stmt>>& stmts) {
        for (const auto& stmt : stmts) {
            if (auto* func_decl = dynamic_cast<FunctionDecl*>(stmt.get())) {
                std::string func_name = func_decl->name;
                if (func_name == "main") {
                    func_name = "sleaf_main";
                    has_main_function = true;
                }

                std::vector<llvm::Type*> param_types;
                for (const auto& param : func_decl->params) {
                    param_types.push_back(get_llvm_type(param.second));
                }

                llvm::Type* ret_type = get_llvm_type(func_decl->return_type);
                llvm::FunctionType* func_type = llvm::FunctionType::get(ret_type, param_types, false);
                llvm::Function* func =
                    llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, func_name, *module);

                // Set names for parameters
                unsigned idx = 0;
                for (auto& arg : func->args()) {
                    arg.setName(func_decl->params[idx++].first);
                }
            }
        }

        for (const auto& stmt : stmts) {
            stmt->accept(*this);
        }

        if (has_main_function) {
            generate_main_wrapper();
        }
    }

    void CodeGenerator::write_to_file(const std::string& filename) {
        std::error_code error_code;
        llvm::raw_fd_ostream out(filename, error_code, llvm::sys::fs::OF_None);
        if (error_code) {
            return;
        }
        module->print(out, nullptr);
        out.close();
    }

    llvm::Type* CodeGenerator::get_llvm_type(TokenType type) {
        switch (type) {
            case TokenType::I8:
                return builder.getInt8Ty();
            case TokenType::I16:
                return builder.getInt16Ty();
            case TokenType::I32:
                return builder.getInt32Ty();
            case TokenType::I64:
                return builder.getInt64Ty();
            case TokenType::U8:
                return builder.getInt8Ty();
            case TokenType::U16:
                return builder.getInt16Ty();
            case TokenType::U32:
                return builder.getInt32Ty();
            case TokenType::U64:
                return builder.getInt64Ty();
            case TokenType::F32:
                return builder.getFloatTy();
            case TokenType::F64:
                return builder.getDoubleTy();
            case TokenType::BOOL:
                return builder.getInt1Ty();
            case TokenType::VOID:
                return builder.getVoidTy();
            default:
                return builder.getInt32Ty();    // Default to i32
        }
    }

    void CodeGenerator::create_entry_block() {
        llvm::BasicBlock* block = llvm::BasicBlock::Create(context, "entry", current_function);
        builder.SetInsertPoint(block);
    }

    void CodeGenerator::generate_main_wrapper() {
        auto* main_func_type =
            llvm::FunctionType::get(builder.getInt32Ty(),
                                    {
                                        builder.getInt32Ty(),    // argc
                                        builder.getInt8Ty()->getPointerTo()->getPointerTo()    // argv
                                    },
                                    false);

        llvm::Function* main_func =
            llvm::Function::Create(main_func_type, llvm::Function::ExternalLinkage, "main", *module);

        // Create entry block
        llvm::BasicBlock* block = llvm::BasicBlock::Create(context, "entry", main_func);
        builder.SetInsertPoint(block);

        // Call user's main function
        llvm::Function* user_main = module->getFunction("sleaf_main");
        if (user_main == nullptr) {
            LOG_ERROR("sleaf_main not found");
            return;
        }

        llvm::Value* ret_val = builder.CreateCall(user_main, {});
        builder.CreateRet(ret_val);
    }

    void CodeGenerator::visit(BlockStmt& node) {
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }

    void CodeGenerator::visit(FunctionDecl& node) {
        std::string func_name = node.name;
        if (node.name == "main") {
            func_name = "sleaf_main";
        }

        current_function = module->getFunction(func_name);
        if (current_function == nullptr) {
            LOG_ERROR("Function not declared: %s", func_name.c_str());
            return;
        }

        create_entry_block();
        named_values.clear();    // Clear local variables

        // Allocate and store parameters
        for (auto& arg : current_function->args()) {
            llvm::AllocaInst* alloca = builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
            builder.CreateStore(&arg, alloca);
            named_values[std::string(arg.getName())] = alloca;
        }

        node.body->accept(*this);

        // Ensure function has terminator
        if (!current_function->getBasicBlockList().back().getTerminator()) {
            if (node.return_type == TokenType::VOID) {
                builder.CreateRetVoid();
            } else {
                LOG_ERROR("Function %s does not return a value", func_name.c_str());
            }
        }

        llvm::verifyFunction(*current_function);
        current_function = nullptr;
    }

    void CodeGenerator::visit(ReturnStmt& node) {
        if (node.value) {
            node.value->accept(*this);
            builder.CreateRet(current_value);
        } else {
            builder.CreateRetVoid();
        }
    }

    void CodeGenerator::visit(Literal& node) {
        switch (node.type) {
            case TokenType::INT_LITERAL:
                current_value = llvm::ConstantInt::get(builder.getInt32Ty(), std::stoi(node.value));
                break;
            case TokenType::FLOAT_LITERAL:
                current_value = llvm::ConstantFP::get(builder.getFloatTy(), std::stof(node.value));
                break;
            case TokenType::F64:
                current_value = llvm::ConstantFP::get(builder.getDoubleTy(), std::stod(node.value));
                break;
            case TokenType::TRUE:
                current_value = llvm::ConstantInt::get(builder.getInt1Ty(), 1);
                break;
            case TokenType::FALSE:
                current_value = llvm::ConstantInt::get(builder.getInt1Ty(), 0);
                break;
            default:
                current_value = llvm::ConstantInt::get(builder.getInt32Ty(), 0);
        }
    }

    void CodeGenerator::visit(ExpressionStmt& node) {
        node.expr->accept(*this);
    }

    void CodeGenerator::visit(BinaryExpr& node) {
        node.left->accept(*this);
        llvm::Value* left = current_value;
        node.right->accept(*this);
        llvm::Value* right = current_value;

        switch (node.op) {
            case TokenType::PLUS:
                if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                    current_value = builder.CreateAdd(left, right, "addtmp");
                } else {
                    current_value = builder.CreateFAdd(left, right, "faddtmp");
                }
                break;
            case TokenType::MINUS:
                if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                    current_value = builder.CreateSub(left, right, "subtmp");
                } else {
                    current_value = builder.CreateFSub(left, right, "fsubtmp");
                }
                break;
            case TokenType::STAR:
                if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                    current_value = builder.CreateMul(left, right, "multmp");
                } else {
                    current_value = builder.CreateFMul(left, right, "fmultmp");
                }
                break;
            case TokenType::SLASH:
                if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy()) {
                    current_value = builder.CreateSDiv(left, right, "divtmp");
                } else {
                    current_value = builder.CreateFDiv(left, right, "fdivtmp");
                }
                break;
            default:
                current_value = nullptr;
        }
    }

    void CodeGenerator::visit(GroupingExpr& node) {
        node.expression->accept(*this);
    }

    void CodeGenerator::visit(Identifier& node) {
        auto iterator = named_values.find(node.name);
        if (iterator != named_values.end()) {
            current_value =
                builder.CreateLoad(iterator->second->getAllocatedType(), iterator->second, node.name.c_str());
        } else {
            LOG_ERROR("Unknown variable: %s", node.name.c_str());
            current_value = nullptr;
        }
    }

    void CodeGenerator::visit(VarDecl& node) {
        if (node.initializer) {
            node.initializer->accept(*this);
            llvm::Value* init_value = current_value;
            llvm::Type* type = get_llvm_type(node.type);

            llvm::AllocaInst* alloca = builder.CreateAlloca(type, nullptr, node.name);
            builder.CreateStore(init_value, alloca);
            named_values[node.name] = alloca;
        }
    }

    void CodeGenerator::visit(Parameter& node) {
        // Parameters are handled in function declaration
    }

    void CodeGenerator::visit(IfStmt& node) {
        node.condition->accept(*this);
        llvm::Value* cond_value = current_value;

        llvm::Function* function = builder.GetInsertBlock()->getParent();
        llvm::BasicBlock* then_block = llvm::BasicBlock::Create(context, "then", function);
        llvm::BasicBlock* else_block = llvm::BasicBlock::Create(context, "else");
        llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "ifcont");

        builder.CreateCondBr(cond_value, then_block, else_block);

        // Then block
        builder.SetInsertPoint(then_block);
        node.then_branch->accept(*this);
        builder.CreateBr(merge_block);

        // Else block
        function->insert(function->end(), else_block);
        builder.SetInsertPoint(else_block);
        if (node.else_branch) {
            node.else_branch->accept(*this);
        }
        builder.CreateBr(merge_block);

        // Merge block
        function->insert(function->end(), merge_block);
        builder.SetInsertPoint(merge_block);
    }

    void CodeGenerator::visit(WhileStmt& node) {
        llvm::Function* function = builder.GetInsertBlock()->getParent();
        llvm::BasicBlock* loop_cond = llvm::BasicBlock::Create(context, "loop_cond", function);
        llvm::BasicBlock* loop_body = llvm::BasicBlock::Create(context, "loop_body", function);
        llvm::BasicBlock* after_loop = llvm::BasicBlock::Create(context, "after_loop", function);

        builder.CreateBr(loop_cond);

        // Condition block
        builder.SetInsertPoint(loop_cond);
        node.condition->accept(*this);
        llvm::Value* cond_value = current_value;
        builder.CreateCondBr(cond_value, loop_body, after_loop);

        // Loop body
        builder.SetInsertPoint(loop_body);
        node.body->accept(*this);
        builder.CreateBr(loop_cond);

        // After loop
        builder.SetInsertPoint(after_loop);
    }

    void CodeGenerator::visit(ForStmt& node) {
        if (node.initializer) {
            node.initializer->accept(*this);
        }

        llvm::Function* function = builder.GetInsertBlock()->getParent();
        llvm::BasicBlock* loop_cond = llvm::BasicBlock::Create(context, "loop_cond", function);
        llvm::BasicBlock* loop_body = llvm::BasicBlock::Create(context, "loop_body", function);
        llvm::BasicBlock* after_loop = llvm::BasicBlock::Create(context, "after_loop", function);

        // Initial jump to condition
        builder.CreateBr(loop_cond);

        // Condition block
        builder.SetInsertPoint(loop_cond);
        if (node.condition) {
            node.condition->accept(*this);
            llvm::Value* cond_value = current_value;
            builder.CreateCondBr(cond_value, loop_body, after_loop);
        } else {
            // No condition - infinite loop
            builder.CreateBr(loop_body);
        }

        // Loop body
        builder.SetInsertPoint(loop_body);
        node.body->accept(*this);

        // Increment
        if (node.increment) {
            node.increment->accept(*this);
        }

        builder.CreateBr(loop_cond);

        // After loop
        builder.SetInsertPoint(after_loop);
    }

    void CodeGenerator::visit(AssignExpr& node) {
        node.value->accept(*this);
        llvm::Value* value = current_value;

        if (auto* identifier = dynamic_cast<Identifier*>(node.target.get())) {
            auto iterator = named_values.find(identifier->name);
            if (iterator != named_values.end()) {
                builder.CreateStore(value, iterator->second);
                current_value = value;
            } else {
                LOG_ERROR("Undefined variable: %s", identifier->name.c_str());
            }
        } else {
            LOG_ERROR("Invalid assignment target");
        }
    }

    void CodeGenerator::visit(UnaryExpr& node) {
        node.operand->accept(*this);
        llvm::Value* operand = current_value;

        switch (node.op) {
            case TokenType::MINUS:
                if (operand->getType()->isIntegerTy()) {
                    current_value = builder.CreateNeg(operand, "negtmp");
                } else {
                    current_value = builder.CreateFNeg(operand, "fnegtmp");
                }
                break;
            case TokenType::BANG:
                current_value = builder.CreateNot(operand, "nottmp");
                break;
            default:
                current_value = nullptr;
        }
    }

    void CodeGenerator::visit(CallExpr& node) {
        std::vector<llvm::Value*> args;
        for (auto& arg : node.arguments) {
            arg->accept(*this);
            args.push_back(current_value);
        }

        node.callee->accept(*this);
        llvm::Value* callee = current_value;

        if (auto* function = llvm::dyn_cast<llvm::Function>(callee)) {
            current_value = builder.CreateCall(function, args, "calltmp");
        } else {
            LOG_ERROR("Call to non-function");
        }
    }
}    // namespace sleaf
