#include <iostream>
#include <stdexcept>
#include <unordered_map>

#include "parser/parser.hpp"

namespace sleaf {

    // Precedence levels for expression parsing
    enum Precedence
    {
        NONE,
        ASSIGNMENT,    // =
        TERNARY,    // ? :
        OR,    // ||
        AND,    // &&
        EQUALITY,    // == !=
        COMPARISON,    // < > <= >=
        TERM,    // + -
        FACTOR,    // * / %
        UNARY,    // ! - ++ --
        CALL,    // . () []
        PRIMARY
    };

    // Precedence table for binary operators
    const std::unordered_map<TokenType, Precedence> PRECEDENCE_TABLE = {
        {TokenType::EQUAL, ASSIGNMENT},
        {TokenType::PLUS_EQUAL, ASSIGNMENT},
        {TokenType::QUESTION, TERNARY},
        {TokenType::PIPE_PIPE, OR},
        {TokenType::AMPERSAND_AMP, AND},
        {TokenType::EQUAL_EQUAL, EQUALITY},
        {TokenType::BANG_EQUAL, EQUALITY},
        {TokenType::LESS, COMPARISON},
        {TokenType::LESS_EQUAL, COMPARISON},
        {TokenType::GREATER, COMPARISON},
        {TokenType::GREATER_EQUAL, COMPARISON},
        {TokenType::PLUS, TERM},
        {TokenType::MINUS, TERM},
        {TokenType::STAR, FACTOR},
        {TokenType::SLASH, FACTOR},
        {TokenType::PERCENT, FACTOR},
        {TokenType::LEFT_PAREN, CALL}};

    Parser::Parser(Lexer& lexer)
        : m_lexer(lexer)
        , m_current(TokenType::END_OF_FILE, "", 0, 0)
        , m_previous(TokenType::END_OF_FILE, "", 0, 0) {
        advance();    // Initialize current token
    }

    auto Parser::parse() -> std::vector<std::unique_ptr<Stmt>> {
        std::vector<std::unique_ptr<Stmt>> statements;
        while (!is_at_end()) {
            statements.push_back(declaration());
        }
        return statements;
    }

    auto Parser::advance() -> void {
        m_previous = m_current;

        if (!is_at_end()) {
            m_current = m_lexer.scan_token();
            if (m_current.type == TokenType::ERROR) {
                error(m_current, m_current.lexeme);
            }
        }
    }

    auto Parser::consume(TokenType type, const std::string& message) -> void {
        if (check(type)) {
            advance();
            return;
        }
        error(m_current, message);
    }

    auto Parser::check(TokenType type) const -> bool {
        if (is_at_end()) {
            return false;
        }
        return m_current.type == type;
    }

    auto Parser::match(TokenType type) -> bool {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    auto Parser::match_any(std::initializer_list<TokenType> types) -> bool {
        for (TokenType type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }

    auto Parser::error(const Token& token, const std::string& message) -> void {
        if (m_panic_mode) {
            return;
        }
        m_panic_mode = true;
        m_error_count++;

        std::cerr << "[Line " << token.line << ", Col " << token.column << "] Error: " << message
                  << std::endl;
    }

    auto Parser::synchronize() -> void {
        m_panic_mode = false;
        synchronize_after_error({TokenType::FUNC,
                                 TokenType::VAR,
                                 TokenType::CONST,
                                 TokenType::FOR,
                                 TokenType::IF,
                                 TokenType::WHILE,
                                 TokenType::RETURN});
    }

    auto Parser::synchronize_after_error(std::initializer_list<TokenType> recovery_tokens) -> void {
        while (!is_at_end()) {
            if (m_previous.type == TokenType::SEMICOLON) {
                return;
            }

            for (TokenType type : recovery_tokens) {
                if (check(type)) {
                    return;
                }
            }

            advance();
        }
    }

    auto Parser::is_at_end() const -> bool {
        return m_current.type == TokenType::END_OF_FILE;
    }

    auto Parser::declaration() -> std::unique_ptr<Stmt> {
        try {
            if (match(TokenType::FUNC)) {
                return function_decl();
            }
            if (match(TokenType::VAR)) {
                return var_declaration(false);
            }
            if (match(TokenType::CONST)) {
                return var_declaration(true);
            }
            return statement();
        } catch (const std::runtime_error& e) {
            synchronize();
            return nullptr;
        }
    }

    auto Parser::function_decl() -> std::unique_ptr<FunctionDecl> {
        consume(TokenType::IDENTIFIER, "Expect function name");
        std::string name = m_previous.lexeme;

        consume(TokenType::LEFT_PAREN, "Expect '(' after function name");
        auto params = parse_parameter_list();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters");

        TokenType return_type = TokenType::VOID;
        if (match(TokenType::ARROW)) {
            return_type = type_annotation();
        }

        auto body = block();
        return std::make_unique<FunctionDecl>(name, params, return_type, std::move(body));
    }

    auto Parser::parse_parameter_list() -> std::vector<std::pair<std::string, TokenType>> {
        std::vector<std::pair<std::string, TokenType>> params;

        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                consume(TokenType::IDENTIFIER, "Expect parameter name");
                std::string param_name = m_previous.lexeme;

                consume(TokenType::COLON, "Expect ':' after parameter name");
                TokenType param_type = type_annotation();

                params.emplace_back(param_name, param_type);
            } while (match(TokenType::COMMA));
        }
        return params;
    }

    auto Parser::statement() -> std::unique_ptr<Stmt> {
        if (match(TokenType::IF)) {
            return if_statement();
        }
        if (match(TokenType::WHILE)) {
            return while_statement();
        }
        if (match(TokenType::FOR)) {
            return for_statement();
        }
        if (match(TokenType::RETURN)) {
            return return_statement();
        }
        if (match(TokenType::LEFT_BRACE)) {
            return block();
        }
        return expression_statement();
    }

    auto Parser::block() -> std::unique_ptr<BlockStmt> {
        std::vector<std::unique_ptr<Stmt>> statements;

        while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
            statements.push_back(declaration());
        }

        consume(TokenType::RIGHT_BRACE, "Expect '}' after block");
        return std::make_unique<BlockStmt>(std::move(statements));
    }

    auto Parser::if_statement() -> std::unique_ptr<Stmt> {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'");
        auto condition = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition");

        auto then_branch = statement();
        std::unique_ptr<Stmt> else_branch = nullptr;

        if (match(TokenType::ELSE)) {
            else_branch = statement();
        }

        return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch));
    }

    auto Parser::while_statement() -> std::unique_ptr<Stmt> {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'");
        auto condition = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition");

        auto body = statement();

        return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
    }

    auto Parser::for_statement() -> std::unique_ptr<Stmt> {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'");

        // Initializer can be var decl or expression
        std::unique_ptr<VarDecl> initializer;
        if (match(TokenType::VAR)) {
            initializer = var_declaration(false);
        } else if (!match(TokenType::SEMICOLON)) {
            auto expr = expression_statement();
            if (auto var_decl = dynamic_cast<VarDecl*>(expr.get())) {
                initializer = std::unique_ptr<VarDecl>(var_decl);
                expr.release();
            } else {
                error(m_previous, "Expect variable declaration in for loop initializer");
            }
        }

        // Condition
        std::unique_ptr<Expr> condition;
        if (!check(TokenType::SEMICOLON)) {
            condition = expression();
        }
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition");

        // Increment
        std::unique_ptr<Expr> increment;
        if (!check(TokenType::RIGHT_PAREN)) {
            increment = expression();
        }
        consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses");

        auto body = statement();

        // Desugar for-loop into while-loop with block
        if (increment) {
            std::vector<std::unique_ptr<Stmt>> body_stmts;
            body_stmts.push_back(std::move(body));
            body_stmts.push_back(std::make_unique<ExpressionStmt>(std::move(increment)));
            body = std::make_unique<BlockStmt>(std::move(body_stmts));
        }

        if (!condition) {
            condition = std::make_unique<Literal>(TokenType::TRUE, "true");
        }

        auto while_loop = std::make_unique<WhileStmt>(std::move(condition), std::move(body));

        // Add initializer if exists
        if (initializer) {
            std::vector<std::unique_ptr<Stmt>> stmts;
            stmts.push_back(std::move(initializer));
            stmts.push_back(std::move(while_loop));
            return std::make_unique<BlockStmt>(std::move(stmts));
        }

        return while_loop;
    }

    auto Parser::var_declaration(bool is_const) -> std::unique_ptr<VarDecl> {
        TokenType type = type_annotation();

        consume(TokenType::IDENTIFIER, "Expect variable name");
        std::string name = m_previous.lexeme;

        std::unique_ptr<Expr> initializer;
        if (match(TokenType::EQUAL)) {
            initializer = expression();
        } else if (is_const) {
            error(m_previous, "Constant must be initialized");
        }

        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");
        return std::make_unique<VarDecl>(type, name, std::move(initializer), is_const);
    }

    auto Parser::return_statement() -> std::unique_ptr<ReturnStmt> {
        std::unique_ptr<Expr> value;
        if (!check(TokenType::SEMICOLON)) {
            value = expression();
        }

        consume(TokenType::SEMICOLON, "Expect ';' after return value");
        return std::make_unique<ReturnStmt>(std::move(value));
    }

    auto Parser::expression_statement() -> std::unique_ptr<ExpressionStmt> {
        auto expr = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after expression");
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }

    auto Parser::expression() -> std::unique_ptr<Expr> {
        return assignment();
    }

    auto Parser::assignment() -> std::unique_ptr<Expr> {
        auto expr = ternary();

        if (match_any({TokenType::EQUAL, TokenType::PLUS_EQUAL})) {
            TokenType op = m_previous.type;
            auto value = assignment();

            if (auto id = dynamic_cast<Identifier*>(expr.get())) {
                return std::make_unique<AssignExpr>(op, std::move(expr), std::move(value));
            }
            error(m_previous, "Invalid assignment target");
        }
        return expr;
    }

    auto Parser::ternary() -> std::unique_ptr<Expr> {
        auto expr = logic_or();

        if (match(TokenType::QUESTION)) {
            auto then_branch = expression();
            consume(TokenType::COLON, "Expect ':' in ternary expression");
            auto else_branch = ternary();

            return std::make_unique<BinaryExpr>(
                TokenType::QUESTION,
                std::move(expr),
                std::make_unique<BinaryExpr>(
                    TokenType::COLON, std::move(then_branch), std::move(else_branch)));
        }
        return expr;
    }

    auto Parser::logic_or() -> std::unique_ptr<Expr> {
        auto expr = logic_and();

        while (match(TokenType::PIPE_PIPE)) {
            TokenType op = m_previous.type;
            auto right = logic_and();
            expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    auto Parser::logic_and() -> std::unique_ptr<Expr> {
        auto expr = equality();

        while (match(TokenType::AMPERSAND_AMP)) {
            TokenType op = m_previous.type;
            auto right = equality();
            expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    auto Parser::equality() -> std::unique_ptr<Expr> {
        auto expr = comparison();

        while (match_any({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
            TokenType op = m_previous.type;
            auto right = comparison();
            expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    auto Parser::comparison() -> std::unique_ptr<Expr> {
        auto expr = term();

        while (
            match_any({TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL}))
        {
            TokenType op = m_previous.type;
            auto right = term();
            expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    auto Parser::term() -> std::unique_ptr<Expr> {
        auto expr = factor();

        while (match_any({TokenType::PLUS, TokenType::MINUS})) {
            TokenType op = m_previous.type;
            auto right = factor();
            expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    auto Parser::factor() -> std::unique_ptr<Expr> {
        auto expr = unary();

        while (match_any({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
            TokenType op = m_previous.type;
            auto right = unary();
            expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
        }
        return expr;
    }

    auto Parser::unary() -> std::unique_ptr<Expr> {
        if (match_any({TokenType::BANG, TokenType::MINUS, TokenType::PLUS_PLUS})) {
            TokenType op = m_previous.type;
            auto operand = unary();
            return std::make_unique<UnaryExpr>(op, std::move(operand));
        }
        return call();
    }

    auto Parser::call() -> std::unique_ptr<Expr> {
        auto expr = primary();

        while (true) {
            if (match(TokenType::LEFT_PAREN)) {
                expr = finish_call(std::move(expr));
            } else {
                break;
            }
        }
        return expr;
    }

    auto Parser::finish_call(std::unique_ptr<Expr> callee) -> std::unique_ptr<Expr> {
        auto arguments = parse_argument_list();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments");

        return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
    }

    auto Parser::parse_argument_list() -> std::vector<std::unique_ptr<Expr>> {
        std::vector<std::unique_ptr<Expr>> arguments;

        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                arguments.push_back(expression());
            } while (match(TokenType::COMMA));
        }
        return arguments;
    }

    auto Parser::primary() -> std::unique_ptr<Expr> {
        if (match(TokenType::FALSE)) {
            return std::make_unique<Literal>(TokenType::FALSE, "false");
        }
        if (match(TokenType::TRUE)) {
            return std::make_unique<Literal>(TokenType::TRUE, "true");
        }
        if (match(TokenType::INT_LITERAL) || match(TokenType::FLOAT_LITERAL)
            || match(TokenType::STRING_LITERAL) || match(TokenType::CHAR_LITERAL))
        {
            return std::make_unique<Literal>(m_previous.type, m_previous.lexeme);
        }
        if (match(TokenType::IDENTIFIER)) {
            return std::make_unique<Identifier>(m_previous.lexeme);
        }
        if (match(TokenType::LEFT_PAREN)) {
            auto expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression");
            return std::make_unique<GroupingExpr>(std::move(expr));
        }

        error(m_current, "Expect expression");
        throw std::runtime_error("Syntax error");
    }

    auto Parser::type_annotation() -> TokenType {
        static const std::unordered_map<std::string, TokenType> type_map = {{"i8", TokenType::I8},
                                                                            {"i16", TokenType::I16},
                                                                            {"i32", TokenType::I32},
                                                                            {"i64", TokenType::I64},
                                                                            {"u8", TokenType::U8},
                                                                            {"u16", TokenType::U16},
                                                                            {"u32", TokenType::U32},
                                                                            {"u64", TokenType::U64},
                                                                            {"f32", TokenType::F32},
                                                                            {"f64", TokenType::F64},
                                                                            {"bool", TokenType::BOOL},
                                                                            {"string", TokenType::STRING},
                                                                            {"char", TokenType::CHAR},
                                                                            {"void", TokenType::VOID}};

        if (m_current.type != TokenType::IDENTIFIER) {
            error(m_current, "Expect type identifier");
            return TokenType::ERROR;
        }

        auto it = type_map.find(m_current.lexeme);
        if (it == type_map.end()) {
            error(m_current, "Unknown type: " + m_current.lexeme);
            return TokenType::ERROR;
        }

        advance();    // Consume type identifier
        return it->second;
    }

}    // namespace sleaf
