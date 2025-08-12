#pragma once

#include <memory>
#include <vector>

#include "lexer/lexer.hpp"

namespace sleaf {

    class Parser;

    struct ASTNode {
        virtual ~ASTNode() = default;
    };

    enum class TypeSpecifier
    {
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        F32,
        F64,
        BOOL,
        CHAR,
        STRING,
        VOID,
        CUSTOM
    };

    struct Type : ASTNode {
        TypeSpecifier base = TypeSpecifier::VOID;
        std::string custom_name;
        bool is_reference = false;
        std::unique_ptr<Type> element_type;
    };

    struct Expr : ASTNode {};

    struct BinaryExpr : Expr {
        TokenType op;
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;
    };

    struct UnaryExpr : Expr {
        TokenType op;
        std::unique_ptr<Expr> operand;
    };

    struct LiteralExpr : Expr {
        TokenType type;
        std::string value;
    };

    struct IdentifierExpr : Expr {
        std::string name;
    };

    struct CallExpr : Expr {
        std::unique_ptr<Expr> callee;
        std::vector<std::unique_ptr<Expr>> arguments;
    };

    struct ArrayAccessExpr : Expr {
        std::unique_ptr<Expr> array;
        std::unique_ptr<Expr> index;
    };

    struct Stmt : ASTNode {};

    struct BlockStmt : Stmt {
        std::vector<std::unique_ptr<Stmt>> statements;
    };

    struct VarDeclStmt : Stmt {
        bool is_const;
        std::unique_ptr<Type> type;
        std::string name;
        std::unique_ptr<Expr> initializer;
    };

    struct ReturnStmt : Stmt {
        std::unique_ptr<Expr> value;
    };

    struct IfStmt : Stmt {
        std::unique_ptr<Expr> condition;
        std::unique_ptr<BlockStmt> then_branch;
        std::unique_ptr<BlockStmt> else_branch;
    };

    struct WhileStmt : Stmt {
        std::unique_ptr<Expr> condition;
        std::unique_ptr<BlockStmt> body;
    };

    struct Declaration : ASTNode {};

    struct FunctionDecl : Declaration {
        std::string name;
        std::vector<std::pair<std::unique_ptr<Type>, std::string>> parameters;
        std::unique_ptr<Type> return_type;
        std::unique_ptr<BlockStmt> body;
    };

    struct StructDecl : Declaration {
        std::string name;
        std::vector<std::pair<std::unique_ptr<Type>, std::string>> members;
    };

    struct Program : ASTNode {
        std::vector<std::unique_ptr<Declaration>> declarations;
    };

    class Parser {
      public:
        explicit Parser(Lexer& lexer);
        std::unique_ptr<Program> parse();
        bool has_errors() const;
        const std::vector<std::string>& get_errors() const;

      private:
        Lexer& lexer;
        Token current;
        Token previous;
        std::vector<std::string> errors;

        void advance();
        void consume(TokenType type, const std::string& message);
        bool match(TokenType type);
        bool check(TokenType type) const;
        bool is_at_end() const;

        std::unique_ptr<Program> parse_program();
        std::unique_ptr<Declaration> parse_declaration();
        std::unique_ptr<FunctionDecl> parse_function();
        std::unique_ptr<StructDecl> parse_struct();
        std::unique_ptr<VarDeclStmt> parse_var_decl();
        std::unique_ptr<Type> parse_type();

        std::unique_ptr<Stmt> parse_statement();
        std::unique_ptr<BlockStmt> parse_block();
        std::unique_ptr<IfStmt> parse_if();
        std::unique_ptr<WhileStmt> parse_while();
        std::unique_ptr<ReturnStmt> parse_return();

        std::unique_ptr<Expr> parse_expression();
        std::unique_ptr<Expr> parse_assignment();
        std::unique_ptr<Expr> parse_equality();
        std::unique_ptr<Expr> parse_comparison();
        std::unique_ptr<Expr> parse_term();
        std::unique_ptr<Expr> parse_factor();
        std::unique_ptr<Expr> parse_unary();
        std::unique_ptr<Expr> parse_primary();

        std::vector<std::pair<std::unique_ptr<Type>, std::string>> parse_parameters();
        std::vector<std::unique_ptr<Expr>> parse_arguments();
    };

}    // namespace sleaf
