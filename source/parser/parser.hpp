/**
 * @file parser.hpp
 * @brief Recursive descent parser for SLEAF language
 *
 * Implements a robust parser with comprehensive error recovery,
 * type annotation, and support for modern language features.
 */

#pragma once

#include <memory>
#include <vector>
#include "lexer/lexer.hpp"
#include "ast/ast.hpp"

namespace sleaf {

/**
 * @class Parser
 * @brief Recursive descent parser with error recovery
 */
class Parser {
public:
    /**
     * @brief Construct a new Parser object
     *
     * @param lexer Lexer instance to provide tokens
     */
    explicit Parser(Lexer& lexer);

    /**
     * @brief Parse the entire program
     * @return std::vector<std::unique_ptr<Stmt>> AST root nodes
     */
    auto parse() -> std::vector<std::unique_ptr<Stmt>>;

    /**
     * @brief Check if parser encountered any errors
     * @return true if errors were detected during parsing
     */
    auto had_error() const -> bool { return m_error_count > 0; }

private:
    Lexer& m_lexer; ///< Reference to lexer
    Token m_current; ///< Current token being processed
    Token m_previous; ///< Previous token processed
    int m_error_count = 0; ///< Number of encountered errors
    bool m_panic_mode = false; ///< Error recovery flag

    // Token handling

    /**
     * @brief Advance to the next token
     */
    auto advance() -> void;

    /**
     * @brief Consume a token of expected type or report error
     * @param type Expected token type
     * @param message Error message to report
     */
    auto consume(TokenType type, const std::string& message) -> void;

    /**
     * @brief Check if current token matches given type
     * @param type Token type to check
     * @return true if current token matches
     */
    auto check(TokenType type) const -> bool;

    /**
     * @brief Match and consume token if it matches given type
     * @param type Token type to match
     * @return true if token matched and consumed
     */
    auto match(TokenType type) -> bool;

    /**
     * @brief Match and consume token if it matches any of given types
     * @param types List of token types to match
     * @return true if any token matched and consumed
     */
    auto match_any(std::initializer_list<TokenType> types) -> bool;

    // Error handling

    /**
     * @brief Report an error at given token
     * @param token Token where error occurred
     * @param message Error description
     */
    auto error(const Token& token, const std::string& message) -> void;

    /**
     * @brief Synchronize parser after error
     */
    auto synchronize() -> void;

    /**
     * @brief Synchronize parser after error until recovery tokens
     * @param recovery_tokens Tokens to recover at
     */
    auto synchronize_after_error(std::initializer_list<TokenType> recovery_tokens) -> void;

    // Grammar productions

    /**
     * @brief Parse a declaration
     * @return Parsed declaration statement
     */
    auto declaration() -> std::unique_ptr<Stmt>;

    /**
     * @brief Parse function declaration
     * @return Parsed function declaration
     */
    auto function_decl() -> std::unique_ptr<FunctionDecl>;

    /**
     * @brief Parse a statement
     * @return Parsed statement
     */
    auto statement() -> std::unique_ptr<Stmt>;

    /**
     * @brief Parse a block statement
     * @return Parsed block statement
     */
    auto block() -> std::unique_ptr<BlockStmt>;

    /**
     * @brief Parse if statement
     * @return Parsed if statement
     */
    auto if_statement() -> std::unique_ptr<Stmt>;

    /**
     * @brief Parse while statement
     * @return Parsed while statement
     */
    auto while_statement() -> std::unique_ptr<Stmt>;

    /**
     * @brief Parse for statement
     * @return Parsed for statement
     */
    auto for_statement() -> std::unique_ptr<Stmt>;

    /**
     * @brief Parse variable declaration
     * @param is_const Whether declaration is constant
     * @return Parsed variable declaration
     */
    auto var_declaration(bool is_const) -> std::unique_ptr<VarDecl>;

    /**
     * @brief Parse return statement
     * @return Parsed return statement
     */
    auto return_statement() -> std::unique_ptr<ReturnStmt>;

    /**
     * @brief Parse expression statement
     * @return Parsed expression statement
     */
    auto expression_statement() -> std::unique_ptr<ExpressionStmt>;

    // Expression parsing

    /**
     * @brief Parse an expression
     * @return Parsed expression
     */
    auto expression() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse assignment expression
     * @return Parsed assignment expression
     */
    auto assignment() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse ternary expression
     * @return Parsed ternary expression
     */
    auto ternary() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse logical OR expression
     * @return Parsed logical OR expression
     */
    auto logic_or() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse logical AND expression
     * @return Parsed logical AND expression
     */
    auto logic_and() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse equality expression
     * @return Parsed equality expression
     */
    auto equality() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse comparison expression
     * @return Parsed comparison expression
     */
    auto comparison() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse term expression
     * @return Parsed term expression
     */
    auto term() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse factor expression
     * @return Parsed factor expression
     */
    auto factor() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse unary expression
     * @return Parsed unary expression
     */
    auto unary() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse function call expression
     * @return Parsed call expression
     */
    auto call() -> std::unique_ptr<Expr>;

    /**
     * @brief Parse primary expression
     * @return Parsed primary expression
     */
    auto primary() -> std::unique_ptr<Expr>;

    // Type handling

    /**
     * @brief Parse type annotation
     * @return Parsed token type
     */
    auto type_annotation() -> TokenType;

    /**
     * @brief Parse parameter list
     * @return Vector of parameter name-type pairs
     */
    auto parse_parameter_list() -> std::vector<std::pair<std::string, TokenType>>;

    /**
     * @brief Parse argument list
     * @return Vector of argument expressions
     */
    auto parse_argument_list() -> std::vector<std::unique_ptr<Expr>>;

    // Helpers

    /**
     * @brief Complete function call parsing
     * @param callee Callee expression
     * @return Completed call expression
     */
    auto finish_call(std::unique_ptr<Expr> callee) -> std::unique_ptr<Expr>;

    /**
     * @brief Check if end of tokens reached
     * @return true if no more tokens
     */
    auto is_at_end() const -> bool;
};

} // namespace sleaf
