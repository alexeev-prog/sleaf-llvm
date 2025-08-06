/**
 * @file lexer.hpp
 * @brief Lexical analyzer for SLEAF programming language
 *
 * Handles tokenization of SLEAF source code with strict typing and modern C-style syntax.
 * Supports all SLEAF types including custom type definitions.
 */

#pragma once

#include <cctype>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sleaf {

    /**
     * @enum TokenType
     * @brief Enumeration of all token types in the SLEAF language
     */
    enum class TokenType
    {
        // Keywords
        FUNC,    ///< "func" keyword
        RETURN,    ///< "return" keyword
        I8,    ///< "i8" type
        I16,    ///< "i16" type
        I32,    ///< "i32" type
        I64,    ///< "i64" type
        U8,    ///< "u8" type
        U16,    ///< "u16" type
        U32,    ///< "u32" type
        U64,    ///< "u64" type
        F32,    ///< "f32" type
        F64,    ///< "f64" type
        BOOL,    ///< "bool" type
        STRING,    ///< "string" type
        CHAR,    ///< "char" type
        IF,    ///< "if" keyword
        ELSE,    ///< "else" keyword
        WHILE,    ///< "while" keyword
        FOR,    ///< "for" keyword
        STRUCT,    ///< "struct" keyword
        IMPORT,    ///< "import" keyword
        CONST,    ///< "const" keyword
        VAR,    ///< "var" keyword
        TRUE,    ///< "true" literal
        FALSE,    ///< "false" literal

        // Identifiers and literals
        IDENTIFIER,    ///< Variable/function name
        INT_LITERAL,    ///< Integer constant
        FLOAT_LITERAL,    ///< Floating-point constant
        STRING_LITERAL,    ///< String constant
        CHAR_LITERAL,    ///< Character constant

        // Operators
        PLUS,    ///< "+"
        MINUS,    ///< "-"
        STAR,    ///< "*"
        SLASH,    ///< "/"
        PERCENT,    ///< "%"
        EQUAL,    ///< "="
        EQUAL_EQUAL,    ///< "=="
        BANG,    ///< "!"
        BANG_EQUAL,    ///< "!="
        LESS,    ///< "<"
        LESS_EQUAL,    ///< "<="
        GREATER,    ///< ">"
        GREATER_EQUAL,    ///< ">="
        AMPERSAND,    ///< "&"
        AMPERSAND_AMP,    ///< "&&"
        PIPE,    ///< "|"
        PIPE_PIPE,    ///< "||"
        ARROW,    ///< "->"
        PLUS_PLUS,    ///< "++"
        PLUS_EQUAL,    ///< "+="

        // Punctuators
        LEFT_PAREN,    ///< "("
        RIGHT_PAREN,    ///< ")"
        LEFT_BRACE,    ///< "{"
        RIGHT_BRACE,    ///< "}"
        LEFT_BRACKET,    ///< "["
        RIGHT_BRACKET,    ///< "]"
        COMMA,    ///< ","
        SEMICOLON,    ///< ";"
        COLON,    ///< ":"
        DOT,    ///< "."

        // Special tokens
        END_OF_FILE,    ///< End of input
        ERROR    ///< Error token
    };

    /**
     * @struct Token
     * @brief Represents a lexical token with metadata
     */
    struct Token {
        TokenType type;    ///< Type of the token
        std::string lexeme;    ///< Original source text of the token
        int line;    ///< Line number where token occurs (1-based)
        int column;    ///< Column number where token starts (1-based)

        /**
         * @brief Construct a new Token object
         *
         * @param t Token type
         * @param l Lexeme string
         * @param ln Line number
         * @param col Column number
         */
        Token(TokenType t, std::string l, int ln, int col)
            : type(t)
            , lexeme(std::move(l))
            , line(ln)
            , column(col) {}

        /**
         * @brief Get the string name of the token type
         * @return std::string Human-readable token type name
         */
        auto type_name() const -> std::string;
    };

    /**
     * @class Lexer
     * @brief Converts SLEAF source code into a sequence of tokens
     */
    class Lexer {
      public:
        /**
         * @brief Construct a new Lexer object
         *
         * @param source Source code to tokenize
         */
        explicit Lexer(std::string source);

        /**
         * @brief Scan the next token from source
         * @return Token Next token in the source
         */
        auto scan_token() -> Token;

        /**
         * @brief Check if lexer reached end of input
         * @return true If no more characters to process
         * @return false If more characters remain
         */
        auto is_at_end() const -> bool;

      private:
        const std::string m_source;    ///< Source code being tokenized
        size_t m_start = 0;    ///< Start of current token
        size_t m_current = 0;    ///< Current position in source
        int m_line = 1;    ///< Current line number (1-based)
        int m_column = 1;    ///< Current column number (1-based)

        /**
         * @brief Advance to next character in source
         * @return char The character before advancing
         */
        auto advance() -> char;

        /**
         * @brief Look at current character without consuming it
         * @return char Current character or '\0' at end
         */
        auto peek() const -> char;

        /**
         * @brief Look at next character without consuming
         * @return char Next character or '\0' if unavailable
         */
        auto peek_next() const -> char;

        /**
         * @brief Conditionally consume character if matches
         *
         * @param expected Character to match
         * @return true If matched and advanced
         * @return false If no match
         */
        auto match(char expected) -> bool;

        /**
         * @brief Create token of given type
         * @param type Type of token to create
         * @return Token Created token
         */
        auto make_token(TokenType type) const -> Token;

        /**
         * @brief Create error token with message
         * @param message Error description
         * @return Token Error token containing message
         */
        auto error_token(const std::string& message) -> Token;

        /**
         * @brief Skip whitespace characters
         */
        auto skip_whitespace() -> void;

        /**
         * @brief Skip single-line comment
         */
        auto skip_line_comment() -> void;

        /**
         * @brief Skip multi-line block comment
         */
        auto skip_block_comment() -> void;

        /**
         * @brief Scan identifier or keyword
         * @return Token Identifier or keyword token
         */
        auto scan_identifier() -> Token;

        /**
         * @brief Scan numeric literal
         * @return Token Integer or float token
         */
        auto scan_number() -> Token;

        /**
         * @brief Scan string literal
         * @return Token String token or error
         */
        auto scan_string() -> Token;

        /**
         * @brief Scan character literal
         * @return Token Char token or error
         */
        auto scan_char() -> Token;

        /**
         * @brief Check if character is alphabetic
         * @param c Character to check
         * @return true If alphabetic character
         */
        auto is_alpha(char c) const -> bool;

        /**
         * @brief Check if character is digit
         * @param c Character to check
         * @return true If decimal digit
         */
        auto is_digit(char c) const -> bool;

        /**
         * @brief Check if character is hex digit
         * @param c Character to check
         * @return true If hexadecimal digit
         */
        auto is_hex_digit(char c) const -> bool;

        /**
         * @brief Check if character is binary digit
         * @param c Character to check
         * @return true If binary digit (0 or 1)
         */
        auto is_bin_digit(char c) const -> bool;

        /**
         * @brief Check if character is alphanumeric
         * @param c Character to check
         * @return true If alphabetic or decimal digit
         */
        auto is_alpha_numeric(char c) const -> bool;
    };

}    // namespace sleaf
