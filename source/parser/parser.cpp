#include <stdexcept>
#include <unordered_map>

#include "parser/parser.hpp"

namespace sleaf {

    Parser::Parser(Lexer& lexer)
        : lexer(lexer)
        , current(TokenType::END_OF_FILE, "", 0, 0)
        , previous(TokenType::END_OF_FILE, "", 0, 0) {
        advance();
        advance();
    }

    std::unique_ptr<Program> Parser::parse() {
        auto program = std::make_unique<Program>();

        while (!is_at_end()) {
            try {
                program->declarations.push_back(parse_declaration());
            } catch (const std::runtime_error& e) {
                while (!is_at_end() && !check(TokenType::FUNC) && !check(TokenType::STRUCT)) {
                    advance();
                }
            }
        }

        return program;
    }

    void Parser::advance() {
        previous = current;

        if (!lexer.is_at_end()) {
            current = lexer.scan_token();
            while (current.type == TokenType::ERROR && !lexer.is_at_end()) {
                current = lexer.scan_token();
            }
        }
    }

    void Parser::consume(TokenType type, const std::string& message) {
        if (check(type)) {
            advance();
            return;
        }
        throw std::runtime_error(message);
    }

    bool Parser::match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool Parser::check(TokenType type) const {
        return current.type == type;
    }

    bool Parser::is_at_end() const {
        return check(TokenType::END_OF_FILE);
    }

    std::unique_ptr<Declaration> Parser::parse_declaration() {
        if (match(TokenType::FUNC)) {
            return parse_function();
        }
        if (match(TokenType::STRUCT)) {
            return parse_struct();
        }
        throw std::runtime_error("Unexpected token at global scope");
    }

    std::unique_ptr<FunctionDecl> Parser::parse_function() {
        auto func = std::make_unique<FunctionDecl>();

        consume(TokenType::IDENTIFIER, "Expect function name");
        func->name = previous.lexeme;

        consume(TokenType::LEFT_PAREN, "Expect '(' after function name");
        func->parameters = parse_parameters();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters");

        consume(TokenType::ARROW, "Expect '->' after function parameters");
        func->return_type = parse_type();

        func->body = parse_block();
        return func;
    }

    std::unique_ptr<StructDecl> Parser::parse_struct() {
        auto struct_decl = std::make_unique<StructDecl>();

        consume(TokenType::IDENTIFIER, "Expect struct name");
        struct_decl->name = previous.lexeme;

        consume(TokenType::LEFT_BRACE, "Expect '{' after struct name");

        while (!check(TokenType::RIGHT_BRACE)) {
            auto member_type = parse_type();
            consume(TokenType::IDENTIFIER, "Expect member name");
            std::string name = previous.lexeme;
            consume(TokenType::SEMICOLON, "Expect ';' after member declaration");

            struct_decl->members.push_back({std::move(member_type), name});
        }

        consume(TokenType::RIGHT_BRACE, "Expect '}' after struct members");
        return struct_decl;
    }

    std::unique_ptr<VarDeclStmt> Parser::parse_var_decl() {
        bool is_const = match(TokenType::CONST);
        if (!is_const) {
            match(TokenType::VAR);
        }

        auto type = parse_type();
        consume(TokenType::IDENTIFIER, "Expect variable name");
        std::string name = previous.lexeme;

        std::unique_ptr<Expr> initializer;
        if (match(TokenType::EQUAL)) {
            initializer = parse_expression();
        }

        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");

        auto var_decl = std::make_unique<VarDeclStmt>();
        var_decl->is_const = is_const;
        var_decl->type = std::move(type);
        var_decl->name = name;
        var_decl->initializer = std::move(initializer);
        return var_decl;
    }

    std::unique_ptr<Type> Parser::parse_type() {
        auto type = std::make_unique<Type>();

        static const std::unordered_map<TokenType, TypeSpecifier> type_map = {
            {TokenType::I8, TypeSpecifier::I8},
            {TokenType::I16, TypeSpecifier::I16},
            {TokenType::I32, TypeSpecifier::I32},
            {TokenType::I64, TypeSpecifier::I64},
            {TokenType::U8, TypeSpecifier::U8},
            {TokenType::U16, TypeSpecifier::U16},
            {TokenType::U32, TypeSpecifier::U32},
            {TokenType::U64, TypeSpecifier::U64},
            {TokenType::F32, TypeSpecifier::F32},
            {TokenType::F64, TypeSpecifier::F64},
            {TokenType::BOOL, TypeSpecifier::BOOL},
            {TokenType::CHAR, TypeSpecifier::CHAR},
            {TokenType::STRING, TypeSpecifier::STRING},
            {TokenType::VOID, TypeSpecifier::VOID}};

        if (auto it = type_map.find(current.type); it != type_map.end()) {
            type->base = it->second;
            advance();
        } else if (match(TokenType::IDENTIFIER)) {
            type->base = TypeSpecifier::CUSTOM;
            type->custom_name = previous.lexeme;
        } else {
            throw std::runtime_error("Expect type specification");
        }

        while (match(TokenType::AMPERSAND)) {
            type->is_reference = true;
        }

        while (match(TokenType::LEFT_BRACKET)) {
            auto element_type = std::make_unique<Type>();
            element_type->base = type->base;
            element_type->custom_name = type->custom_name;
            element_type->is_reference = type->is_reference;
            element_type->element_type = std::move(type->element_type);

            type->base = TypeSpecifier::CUSTOM;
            type->custom_name = "array";
            type->element_type = std::move(element_type);
            type->is_reference = false;

            consume(TokenType::RIGHT_BRACKET, "Expect ']' after array type");
        }

        return type;
    }

    std::vector<std::pair<std::unique_ptr<Type>, std::string>> Parser::parse_parameters() {
        std::vector<std::pair<std::unique_ptr<Type>, std::string>> params;

        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                auto type = parse_type();
                consume(TokenType::IDENTIFIER, "Expect parameter name");
                params.push_back({std::move(type), previous.lexeme});
            } while (match(TokenType::COMMA));
        }

        return params;
    }

    std::unique_ptr<Stmt> Parser::parse_statement() {
        if (match(TokenType::LEFT_BRACE)) {
            return parse_block();
        }
        if (match(TokenType::IF)) {
            return parse_if();
        }
        if (match(TokenType::WHILE)) {
            return parse_while();
        }
        if (match(TokenType::RETURN)) {
            return parse_return();
        }
        if (match(TokenType::VAR) || match(TokenType::CONST)) {
            return parse_var_decl();
        }

        auto expr = parse_expression();
        consume(TokenType::SEMICOLON, "Expect ';' after expression");
        // return expr;
    }

    std::unique_ptr<BlockStmt> Parser::parse_block() {
        auto block = std::make_unique<BlockStmt>();

        while (!check(TokenType::RIGHT_BRACE)) {
            block->statements.push_back(parse_statement());
        }

        consume(TokenType::RIGHT_BRACE, "Expect '}' after block");
        return block;
    }

    std::unique_ptr<IfStmt> Parser::parse_if() {
        auto if_stmt = std::make_unique<IfStmt>();

        consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'");
        if_stmt->condition = parse_expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after condition");

        if_stmt->then_branch = parse_block();

        if (match(TokenType::ELSE)) {
            if_stmt->else_branch = parse_block();
        }

        return if_stmt;
    }

    std::unique_ptr<WhileStmt> Parser::parse_while() {
        auto while_stmt = std::make_unique<WhileStmt>();

        consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'");
        while_stmt->condition = parse_expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after condition");

        while_stmt->body = parse_block();
        return while_stmt;
    }

    std::unique_ptr<ReturnStmt> Parser::parse_return() {
        auto return_stmt = std::make_unique<ReturnStmt>();

        if (!check(TokenType::SEMICOLON)) {
            return_stmt->value = parse_expression();
        }

        consume(TokenType::SEMICOLON, "Expect ';' after return");
        return return_stmt;
    }

    std::unique_ptr<Expr> Parser::parse_expression() {
        return parse_assignment();
    }

    std::unique_ptr<Expr> Parser::parse_assignment() {
        auto expr = parse_equality();

        if (match(TokenType::EQUAL)) {
            auto value = parse_assignment();
            auto assign = std::make_unique<BinaryExpr>();
            assign->left = std::move(expr);
            assign->op = TokenType::EQUAL;
            assign->right = std::move(value);
            expr = std::move(assign);
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parse_equality() {
        auto expr = parse_comparison();

        while (match(TokenType::BANG_EQUAL) || match(TokenType::EQUAL_EQUAL)) {
            auto bin_expr = std::make_unique<BinaryExpr>();
            bin_expr->left = std::move(expr);
            bin_expr->op = previous.type;
            bin_expr->right = parse_comparison();
            expr = std::move(bin_expr);
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parse_comparison() {
        auto expr = parse_term();

        while (match(TokenType::LESS) || match(TokenType::LESS_EQUAL) || match(TokenType::GREATER)
               || match(TokenType::GREATER_EQUAL))
        {
            auto bin_expr = std::make_unique<BinaryExpr>();
            bin_expr->left = std::move(expr);
            bin_expr->op = previous.type;
            bin_expr->right = parse_term();
            expr = std::move(bin_expr);
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parse_term() {
        auto expr = parse_factor();

        while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
            auto bin_expr = std::make_unique<BinaryExpr>();
            bin_expr->left = std::move(expr);
            bin_expr->op = previous.type;
            bin_expr->right = parse_factor();
            expr = std::move(bin_expr);
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parse_factor() {
        auto expr = parse_unary();

        while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::PERCENT)) {
            auto bin_expr = std::make_unique<BinaryExpr>();
            bin_expr->left = std::move(expr);
            bin_expr->op = previous.type;
            bin_expr->right = parse_unary();
            expr = std::move(bin_expr);
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parse_unary() {
        if (match(TokenType::BANG) || match(TokenType::MINUS)) {
            auto unary = std::make_unique<UnaryExpr>();
            unary->op = previous.type;
            unary->operand = parse_unary();
            return unary;
        }

        return parse_primary();
    }

    std::unique_ptr<Expr> Parser::parse_primary() {
        if (match(TokenType::TRUE) || match(TokenType::FALSE) || match(TokenType::INT_LITERAL)
            || match(TokenType::FLOAT_LITERAL) || match(TokenType::STRING_LITERAL)
            || match(TokenType::CHAR_LITERAL))
        {
            auto literal = std::make_unique<LiteralExpr>();
            literal->type = previous.type;
            literal->value = previous.lexeme;
            return literal;
        }

        if (match(TokenType::IDENTIFIER)) {
            auto ident = std::make_unique<IdentifierExpr>();
            ident->name = previous.lexeme;

            if (match(TokenType::LEFT_PAREN)) {
                auto call = std::make_unique<CallExpr>();
                call->callee = std::move(ident);
                call->arguments = parse_arguments();
                return call;
            }

            if (match(TokenType::LEFT_BRACKET)) {
                auto access = std::make_unique<ArrayAccessExpr>();
                access->array = std::move(ident);
                access->index = parse_expression();
                consume(TokenType::RIGHT_BRACKET, "Expect ']' after index");
                return access;
            }

            return ident;
        }

        if (match(TokenType::LEFT_PAREN)) {
            auto expr = parse_expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression");
            return expr;
        }

        throw std::runtime_error("Expect expression");
    }

    std::vector<std::unique_ptr<Expr>> Parser::parse_arguments() {
        std::vector<std::unique_ptr<Expr>> args;

        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                args.push_back(parse_expression());
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments");
        return args;
    }

}    // namespace sleaf
