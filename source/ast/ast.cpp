#include "ast/ast.hpp"

namespace sleaf {

    // BlockStmt implementation
    BlockStmt::BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}

    void BlockStmt::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // FunctionDecl implementation
    FunctionDecl::FunctionDecl(std::string name,
                               std::vector<std::pair<std::string, TokenType>> params,
                               TokenType return_type,
                               std::unique_ptr<BlockStmt> body)
        : name(std::move(name))
        , params(std::move(params))
        , return_type(return_type)
        , body(std::move(body)) {}

    void FunctionDecl::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // VarDecl implementation
    VarDecl::VarDecl(TokenType type, std::string name, std::unique_ptr<Expr> initializer, bool is_const)
        : type(type)
        , name(std::move(name))
        , initializer(std::move(initializer))
        , is_const(is_const) {}

    void VarDecl::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // Parameter implementation
    Parameter::Parameter(std::string name, TokenType type)
        : name(std::move(name))
        , type(type) {}

    void Parameter::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // IfStmt implementation
    IfStmt::IfStmt(std::unique_ptr<Expr> condition,
                   std::unique_ptr<Stmt> then_branch,
                   std::unique_ptr<Stmt> else_branch)
        : condition(std::move(condition))
        , then_branch(std::move(then_branch))
        , else_branch(std::move(else_branch)) {}

    void IfStmt::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // WhileStmt implementation
    WhileStmt::WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition))
        , body(std::move(body)) {}

    void WhileStmt::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // ForStmt implementation
    ForStmt::ForStmt(std::unique_ptr<VarDecl> initializer,
                     std::unique_ptr<Expr> condition,
                     std::unique_ptr<Expr> increment,
                     std::unique_ptr<Stmt> body)
        : initializer(std::move(initializer))
        , condition(std::move(condition))
        , increment(std::move(increment))
        , body(std::move(body)) {}

    void ForStmt::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // ReturnStmt implementation
    ReturnStmt::ReturnStmt(std::unique_ptr<Expr> value)
        : value(std::move(value)) {}

    void ReturnStmt::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // ExpressionStmt implementation
    ExpressionStmt::ExpressionStmt(std::unique_ptr<Expr> expr)
        : expr(std::move(expr)) {}

    void ExpressionStmt::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    // BinaryExpr implementation
    BinaryExpr::BinaryExpr(TokenType op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : op(op)
        , left(std::move(left))
        , right(std::move(right)) {}

    void BinaryExpr::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    auto BinaryExpr::get_type() const -> TokenType {
        // Type propagation rules
        if (left->get_type() == TokenType::F32 || right->get_type() == TokenType::F32
            || left->get_type() == TokenType::F64 || right->get_type() == TokenType::F64)
        {
            return TokenType::F64;
        }
        return TokenType::I32;    // Default to largest integer type
    }

    // AssignExpr implementation
    AssignExpr::AssignExpr(TokenType op, std::unique_ptr<Expr> target, std::unique_ptr<Expr> value)
        : op(op)
        , target(std::move(target))
        , value(std::move(value)) {}

    void AssignExpr::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    TokenType AssignExpr::get_type() const {
        return target->get_type();
    }

    // UnaryExpr implementation
    UnaryExpr::UnaryExpr(TokenType op, std::unique_ptr<Expr> operand)
        : op(op)
        , operand(std::move(operand)) {}

    void UnaryExpr::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    TokenType UnaryExpr::get_type() const {
        return operand->get_type();
    }

    // CallExpr implementation
    CallExpr::CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee))
        , arguments(std::move(arguments)) {}

    void CallExpr::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    TokenType CallExpr::get_type() const {
        // In real implementation, would look up function return type
        return TokenType::I32;    // Default return type
    }

    // Identifier implementation
    Identifier::Identifier(std::string name)
        : name(std::move(name)) {}

    void Identifier::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    TokenType Identifier::get_type() const {
        // In real implementation, would look up in symbol table
        return TokenType::I32;    // Default type
    }

    // Literal implementation
    Literal::Literal(TokenType type, std::string value)
        : type(type)
        , value(std::move(value)) {}

    void Literal::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    TokenType Literal::get_type() const {
        return type;
    }

    // GroupingExpr implementation
    GroupingExpr::GroupingExpr(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    void GroupingExpr::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    TokenType GroupingExpr::get_type() const {
        return expression->get_type();
    }

}    // namespace sleaf
