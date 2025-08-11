// codegen.hpp
#pragma once

#include <map>
#include <memory>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "ast/ast.hpp"

namespace sleaf {

    class CodeGenerator : public ASTVisitor {
      public:
        CodeGenerator();
        ~CodeGenerator() = default;

        void generate(const std::vector<std::unique_ptr<Stmt>>& stmts);
        void write_to_file(const std::string& filename);

        void visit(BlockStmt& node) override;
        void visit(FunctionDecl& node) override;
        void visit(VarDecl& node) override;
        void visit(Parameter& node) override;
        void visit(IfStmt& node) override;
        void visit(WhileStmt& node) override;
        void visit(ForStmt& node) override;
        void visit(ReturnStmt& node) override;
        void visit(ExpressionStmt& node) override;
        void visit(BinaryExpr& node) override;
        void visit(AssignExpr& node) override;
        void visit(UnaryExpr& node) override;
        void visit(CallExpr& node) override;
        void visit(Identifier& node) override;
        void visit(Literal& node) override;
        void visit(GroupingExpr& node) override;

      private:
        llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> module;
        llvm::IRBuilder<> builder;
        std::map<std::string, llvm::Value*> named_values;

        llvm::Function* current_function = nullptr;
        llvm::Value* current_value = nullptr;
        bool has_main_function = false;

        llvm::Type* get_llvm_type(TokenType type);
        void create_entry_block();
        void generate_main_wrapper();
    };

}    // namespace sleaf
