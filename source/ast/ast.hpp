/**
 * @file ast.hpp
 * @brief Abstract Syntax Tree (AST) for SLEAF programming language
 *
 * Defines comprehensive AST node structure with visitor pattern support
 * for various language constructs including functions, control flow,
 * variables, and expressions.
 */

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "lexer/lexer.hpp"

namespace sleaf {

    // Forward declarations
    class ASTVisitor;
    class Expr;
    class Stmt;

    /**
     * @class ASTNode
     * @brief Base interface for all AST nodes
     */
    class ASTNode {
      public:
        virtual ~ASTNode() = default;

        /**
         * @brief Accept visitor for traversal
         * @param visitor Reference to visitor implementation
         */
        virtual auto accept(ASTVisitor& visitor) -> void = 0;
    };

    /**
     * @class Expr
     * @brief Base class for all expression nodes
     */
    class Expr : public ASTNode {
      public:
        /**
         * @brief Get expression type for type checking
         * @return TokenType representing expression type
         */
        virtual auto get_type() const -> TokenType = 0;
    };

    /**
     * @class Stmt
     * @brief Base class for all statement nodes
     */
    class Stmt : public ASTNode {};

    /**
     * @class BlockStmt
     * @brief Represents a block of statements
     */
    class BlockStmt : public Stmt {
      public:
        std::vector<std::unique_ptr<Stmt>> statements;

        explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class FunctionDecl
     * @brief Represents function declaration
     */
    class FunctionDecl : public Stmt {
      public:
        std::string name;
        std::vector<std::pair<std::string, TokenType>> params;
        TokenType return_type;
        std::unique_ptr<BlockStmt> body;

        FunctionDecl(std::string name,
                     std::vector<std::pair<std::string, TokenType>> params,
                     TokenType return_type,
                     std::unique_ptr<BlockStmt> body);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class VarDecl
     * @brief Represents variable declaration
     */
    class VarDecl : public Stmt {
      public:
        TokenType type;
        std::string name;
        std::unique_ptr<Expr> initializer;
        bool is_const;

        VarDecl(TokenType type, std::string name, std::unique_ptr<Expr> initializer, bool is_const);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class Parameter
     * @brief Represents function parameter declaration
     */
    class Parameter : public ASTNode {
      public:
        std::string name;
        TokenType type;

        Parameter(std::string name, TokenType type);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class IfStmt
     * @brief Represents if/else conditional statement
     */
    class IfStmt : public Stmt {
      public:
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Stmt> then_branch;
        std::unique_ptr<Stmt> else_branch;

        IfStmt(std::unique_ptr<Expr> condition,
               std::unique_ptr<Stmt> then_branch,
               std::unique_ptr<Stmt> else_branch);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class WhileStmt
     * @brief Represents while loop statement
     */
    class WhileStmt : public Stmt {
      public:
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Stmt> body;

        WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class ForStmt
     * @brief Represents for loop statement
     */
    class ForStmt : public Stmt {
      public:
        std::unique_ptr<VarDecl> initializer;
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Expr> increment;
        std::unique_ptr<Stmt> body;

        ForStmt(std::unique_ptr<VarDecl> initializer,
                std::unique_ptr<Expr> condition,
                std::unique_ptr<Expr> increment,
                std::unique_ptr<Stmt> body);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class ReturnStmt
     * @brief Represents return statement
     */
    class ReturnStmt : public Stmt {
      public:
        std::unique_ptr<Expr> value;

        explicit ReturnStmt(std::unique_ptr<Expr> value);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class ExpressionStmt
     * @brief Represents expression as statement
     */
    class ExpressionStmt : public Stmt {
      public:
        std::unique_ptr<Expr> expr;

        explicit ExpressionStmt(std::unique_ptr<Expr> expr);
        auto accept(ASTVisitor& visitor) -> void override;
    };

    /**
     * @class BinaryExpr
     * @brief Represents binary operation expression
     */
    class BinaryExpr : public Expr {
      public:
        TokenType op;
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;

        BinaryExpr(TokenType op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);
        auto accept(ASTVisitor& visitor) -> void override;
        auto get_type() const -> TokenType override;
    };

    /**
     * @class AssignExpr
     * @brief Represents assignment expression
     */
    class AssignExpr : public Expr {
      public:
        TokenType op;
        std::unique_ptr<Expr> target;
        std::unique_ptr<Expr> value;

        AssignExpr(TokenType op, std::unique_ptr<Expr> target, std::unique_ptr<Expr> value);
        auto accept(ASTVisitor& visitor) -> void override;
        auto get_type() const -> TokenType override;
    };

    /**
     * @class UnaryExpr
     * @brief Represents unary operation expression
     */
    class UnaryExpr : public Expr {
      public:
        TokenType op;
        std::unique_ptr<Expr> operand;

        UnaryExpr(TokenType op, std::unique_ptr<Expr> operand);
        auto accept(ASTVisitor& visitor) -> void override;
        auto get_type() const -> TokenType override;
    };

    /**
     * @class CallExpr
     * @brief Represents function call expression
     */
    class CallExpr : public Expr {
      public:
        std::unique_ptr<Expr> callee;
        std::vector<std::unique_ptr<Expr>> arguments;

        CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments);
        auto accept(ASTVisitor& visitor) -> void override;
        auto get_type() const -> TokenType override;
    };

    /**
     * @class Identifier
     * @brief Represents identifier expression
     */
    class Identifier : public Expr {
      public:
        std::string name;

        explicit Identifier(std::string name);
        auto accept(ASTVisitor& visitor) -> void override;
        auto get_type() const -> TokenType override;
    };

    /**
     * @class Literal
     * @brief Represents literal value expression
     */
    class Literal : public Expr {
      public:
        TokenType type;
        std::string value;

        Literal(TokenType type, std::string value);
        auto accept(ASTVisitor& visitor) -> void override;
        auto get_type() const -> TokenType override;
    };

    /**
     * @class GroupingExpr
     * @brief Represents parenthesized expression
     */
    class GroupingExpr : public Expr {
      public:
        std::unique_ptr<Expr> expression;

        explicit GroupingExpr(std::unique_ptr<Expr> expression);
        auto accept(ASTVisitor& visitor) -> void override;
        auto get_type() const -> TokenType override;
    };

    /**
     * @class ASTVisitor
     * @brief Visitor interface for AST traversal
     */
    class ASTVisitor {
      public:
        virtual ~ASTVisitor() = default;

        // Statement visitors
        virtual void visit(BlockStmt& node) = 0;
        virtual void visit(FunctionDecl& node) = 0;
        virtual void visit(VarDecl& node) = 0;
        virtual void visit(Parameter& node) = 0;
        virtual void visit(IfStmt& node) = 0;
        virtual void visit(WhileStmt& node) = 0;
        virtual void visit(ForStmt& node) = 0;
        virtual void visit(ReturnStmt& node) = 0;
        virtual void visit(ExpressionStmt& node) = 0;

        // Expression visitors
        virtual void visit(BinaryExpr& node) = 0;
        virtual void visit(AssignExpr& node) = 0;
        virtual void visit(UnaryExpr& node) = 0;
        virtual void visit(CallExpr& node) = 0;
        virtual void visit(Identifier& node) = 0;
        virtual void visit(Literal& node) = 0;
        virtual void visit(GroupingExpr& node) = 0;
    };

}    // namespace sleaf
