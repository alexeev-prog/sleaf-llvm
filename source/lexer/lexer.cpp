#include <cctype>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "lexer/lexer.hpp"

namespace sleaf {

    auto Token::type_name() const -> std::string {
        static const std::unordered_map<TokenType, std::string> names = {
            {TokenType::FUNC, "FUNC"},
            {TokenType::RETURN, "RETURN"},
            {TokenType::I8, "I8"},
            {TokenType::I16, "I16"},
            {TokenType::I32, "I32"},
            {TokenType::I64, "I64"},
            {TokenType::U8, "U8"},
            {TokenType::U16, "U16"},
            {TokenType::U32, "U32"},
            {TokenType::U64, "U64"},
            {TokenType::F32, "F32"},
            {TokenType::F64, "F64"},
            {TokenType::BOOL, "BOOL"},
            {TokenType::STRING, "STRING"},
            {TokenType::CHAR, "CHAR"},
            {TokenType::VOID, "VOID"},
            {TokenType::IF, "IF"},
            {TokenType::ELSE, "ELSE"},
            {TokenType::WHILE, "WHILE"},
            {TokenType::FOR, "FOR"},
            {TokenType::STRUCT, "STRUCT"},
            {TokenType::IMPORT, "IMPORT"},
            {TokenType::CONST, "CONST"},
            {TokenType::VAR, "VAR"},
            {TokenType::TRUE, "TRUE"},
            {TokenType::FALSE, "FALSE"},
            {TokenType::IDENTIFIER, "IDENTIFIER"},
            {TokenType::INT_LITERAL, "INT_LITERAL"},
            {TokenType::FLOAT_LITERAL, "FLOAT_LITERAL"},
            {TokenType::STRING_LITERAL, "STRING_LITERAL"},
            {TokenType::CHAR_LITERAL, "CHAR_LITERAL"},
            {TokenType::PLUS, "PLUS"},
            {TokenType::MINUS, "MINUS"},
            {TokenType::STAR, "STAR"},
            {TokenType::SLASH, "SLASH"},
            {TokenType::PERCENT, "PERCENT"},
            {TokenType::EQUAL, "EQUAL"},
            {TokenType::EQUAL_EQUAL, "EQUAL_EQUAL"},
            {TokenType::BANG, "BANG"},
            {TokenType::BANG_EQUAL, "BANG_EQUAL"},
            {TokenType::LESS, "LESS"},
            {TokenType::LESS_EQUAL, "LESS_EQUAL"},
            {TokenType::GREATER, "GREATER"},
            {TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
            {TokenType::AMPERSAND, "AMPERSAND"},
            {TokenType::AMPERSAND_AMP, "AMPERSAND_AMP"},
            {TokenType::PIPE, "PIPE"},
            {TokenType::PIPE_PIPE, "PIPE_PIPE"},
            {TokenType::ARROW, "ARROW"},
            {TokenType::PLUS_PLUS, "PLUS_PLUS"},
            {TokenType::PLUS_EQUAL, "PLUS_EQUAL"},
            {TokenType::LEFT_PAREN, "LEFT_PAREN"},
            {TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
            {TokenType::LEFT_BRACE, "LEFT_BRACE"},
            {TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
            {TokenType::LEFT_BRACKET, "LEFT_BRACKET"},
            {TokenType::RIGHT_BRACKET, "RIGHT_BRACKET"},
            {TokenType::COMMA, "COMMA"},
            {TokenType::SEMICOLON, "SEMICOLON"},
            {TokenType::COLON, "COLON"},
            {TokenType::DOT, "DOT"},
            {TokenType::QUESTION, "QUESTION"},
            {TokenType::END_OF_FILE, "END_OF_FILE"},
            {TokenType::ERROR, "ERROR"}};

        auto it = names.find(type);
        return it != names.end() ? it->second : "UNKNOWN";
    }

    Lexer::Lexer(std::string source)
        : m_SOURCE(std::move(source)) {}

    auto Lexer::scan_token() -> Token {
        skip_whitespace();

        if (is_at_end()) {
            return make_token(TokenType::END_OF_FILE);
        }

        m_START = m_CURRENT;
        char c = advance();

        if (is_alpha(c)) {
            return scan_identifier();
        }
        if (is_digit(c)) {
            return scan_number();
        }

        switch (c) {
            case '(':
                return make_token(TokenType::LEFT_PAREN);
            case ')':
                return make_token(TokenType::RIGHT_PAREN);
            case '{':
                return make_token(TokenType::LEFT_BRACE);
            case '}':
                return make_token(TokenType::RIGHT_BRACE);
            case '[':
                return make_token(TokenType::LEFT_BRACKET);
            case ']':
                return make_token(TokenType::RIGHT_BRACKET);
            case ',':
                return make_token(TokenType::COMMA);
            case ';':
                return make_token(TokenType::SEMICOLON);
            case ':':
                return make_token(TokenType::COLON);
            case '.':
                return make_token(TokenType::DOT);
            case '?':
                return make_token(TokenType::QUESTION);
            case '+':
                if (match('+')) {
                    return make_token(TokenType::PLUS_PLUS);
                }
                if (match('=')) {
                    return make_token(TokenType::PLUS_EQUAL);
                }
                return make_token(TokenType::PLUS);
            case '-':
                if (match('>')) {
                    return make_token(TokenType::ARROW);
                }
                return make_token(TokenType::MINUS);
            case '*':
                return make_token(TokenType::STAR);
            case '/':
                if (match('/')) {
                    skip_line_comment();
                    return scan_token();
                }
                if (match('*')) {
                    skip_block_comment();
                    return scan_token();
                }
                return make_token(TokenType::SLASH);
            case '%':
                return make_token(TokenType::PERCENT);
            case '=':
                if (match('=')) {
                    return make_token(TokenType::EQUAL_EQUAL);
                }
                return make_token(TokenType::EQUAL);
            case '!':
                if (match('=')) {
                    return make_token(TokenType::BANG_EQUAL);
                }
                return make_token(TokenType::BANG);
            case '<':
                if (match('=')) {
                    return make_token(TokenType::LESS_EQUAL);
                }
                return make_token(TokenType::LESS);
            case '>':
                if (match('=')) {
                    return make_token(TokenType::GREATER_EQUAL);
                }
                return make_token(TokenType::GREATER);
            case '&':
                if (match('&')) {
                    return make_token(TokenType::AMPERSAND_AMP);
                }
                return make_token(TokenType::AMPERSAND);
            case '|':
                if (match('|')) {
                    return make_token(TokenType::PIPE_PIPE);
                }
                return make_token(TokenType::PIPE);
            case '"':
                return scan_string();
            case '\'':
                return scan_char();
            default:
                return error_token("Unexpected character: " + std::string(1, c));
        }
    }

    auto Lexer::is_at_end() const -> bool {
        return m_CURRENT >= m_SOURCE.length();
    }

    auto Lexer::advance() -> char {
        if (is_at_end()) {
            return '\0';
        }
        char c = m_SOURCE[m_CURRENT++];
        if (c == '\n') {
            m_LINE++;
            m_COLUMN = 1;
        } else {
            m_COLUMN++;
        }
        return c;
    }

    auto Lexer::peek() const -> char {
        return is_at_end() ? '\0' : m_SOURCE[m_CURRENT];
    }

    auto Lexer::peek_next() const -> char {
        if (m_CURRENT + 1 >= m_SOURCE.length()) {
            return '\0';
        }
        return m_SOURCE[m_CURRENT + 1];
    }

    auto Lexer::match(char expected) -> bool {
        if (is_at_end()) {
            return false;
        }
        if (m_SOURCE[m_CURRENT] != expected) {
            return false;
        }

        advance();
        return true;
    }

    auto Lexer::make_token(TokenType type) const -> Token {
        std::string lexeme = m_SOURCE.substr(m_START, m_CURRENT - m_START);
        return {type, lexeme, m_LINE, m_COLUMN - static_cast<int>(lexeme.length())};
    }

    auto Lexer::error_token(const std::string& message) -> Token {
        return {TokenType::ERROR, message, m_LINE, m_COLUMN};
    }

    auto Lexer::skip_whitespace() -> void {
        while (!is_at_end()) {
            char c = peek();
            switch (c) {
                case ' ':
                case '\r':
                case '\t':
                    advance();
                    break;
                case '\n':
                    advance();
                    break;
                default:
                    return;
            }
        }
    }

    auto Lexer::skip_line_comment() -> void {
        while (peek() != '\n' && !is_at_end()) {
            advance();
        }
    }

    auto Lexer::skip_block_comment() -> void {
        while (!is_at_end()) {
            if (peek() == '*' && peek_next() == '/') {
                advance();    // Skip *
                advance();    // Skip /
                return;
            }
            advance();
        }
    }

    auto Lexer::scan_identifier() -> Token {
        while (is_alpha_numeric(peek())) {
            advance();
        }

        static const std::unordered_map<std::string, TokenType> keywords = {
            {"func", TokenType::FUNC},   {"return", TokenType::RETURN}, {"i8", TokenType::I8},
            {"i16", TokenType::I16},     {"i32", TokenType::I32},       {"i64", TokenType::I64},
            {"u8", TokenType::U8},       {"u16", TokenType::U16},       {"u32", TokenType::U32},
            {"u64", TokenType::U64},     {"f32", TokenType::F32},       {"f64", TokenType::F64},
            {"bool", TokenType::BOOL},   {"string", TokenType::STRING}, {"char", TokenType::CHAR},
            {"void", TokenType::VOID},   {"true", TokenType::TRUE},     {"false", TokenType::FALSE},
            {"if", TokenType::IF},       {"else", TokenType::ELSE},     {"while", TokenType::WHILE},
            {"for", TokenType::FOR},     {"struct", TokenType::STRUCT}, {"import", TokenType::IMPORT},
            {"const", TokenType::CONST}, {"var", TokenType::VAR}};

        std::string text = m_SOURCE.substr(m_START, m_CURRENT - m_START);
        auto it = keywords.find(text);
        if (it != keywords.end()) {
            return make_token(it->second);
        }
        return make_token(TokenType::IDENTIFIER);
    }

    auto Lexer::scan_number() -> Token {
        bool is_float = false;
        bool is_hex = false;
        bool is_bin = false;

        // Check for hex or binary prefix
        if (peek() == 'x' && (m_CURRENT - m_START == 1) && m_SOURCE[m_START] == '0') {
            is_hex = true;
            advance();    // Skip 'x'
        } else if (peek() == 'b' && (m_CURRENT - m_START == 1) && m_SOURCE[m_START] == '0') {
            is_bin = true;
            advance();    // Skip 'b'
        }

        while (!is_at_end()) {
            char c = peek();
            if (c == '.') {
                if (is_float || is_hex || is_bin) {
                    return error_token("Invalid numeric format");
                }
                is_float = true;
                advance();
            } else if (c == '_') {
                advance();    // Skip underscores in numbers
            } else if (is_hex ? is_hex_digit(c) : is_bin ? is_bin_digit(c) : is_digit(c)) {
                advance();
            } else {
                break;
            }
        }

        // Handle exponent part for floats
        if (!is_hex && !is_bin && (peek() == 'e' || peek() == 'E')) {
            is_float = true;
            advance();    // Skip 'e' or 'E'

            if (peek() == '+' || peek() == '-') {
                advance();    // Skip sign
            }

            while (is_digit(peek())) {
                advance();
            }
        }

        return make_token(is_float ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL);
    }

    auto Lexer::scan_string() -> Token {
        while (!is_at_end() && peek() != '"') {
            if (peek() == '\\') {
                advance();    // Skip escape character
            }
            advance();
        }

        if (is_at_end()) {
            return error_token("Unterminated string");
        }

        advance();    // Skip closing quote
        return make_token(TokenType::STRING_LITERAL);
    }

    auto Lexer::scan_char() -> Token {
        advance();

        if (is_at_end()) {
            return error_token("Unterminated character");
        }

        char c = peek();
        if (c == '\\') {
            advance();
            if (is_at_end()) {
                return error_token("Unterminated character after escape");
            }
            advance();
        } else {
            advance();
        }

        if (peek() != '\'') {
            return error_token("Character too long");
        }

        advance();
        return make_token(TokenType::CHAR_LITERAL);
    }

    auto Lexer::is_alpha(char c) const -> bool {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'
            || static_cast<unsigned char>(c) > 0x7F;
    }

    auto Lexer::is_digit(char c) const -> bool {
        return c >= '0' && c <= '9';
    }

    auto Lexer::is_hex_digit(char c) const -> bool {
        return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    auto Lexer::is_bin_digit(char c) const -> bool {
        return c == '0' || c == '1';
    }

    auto Lexer::is_alpha_numeric(char c) const -> bool {
        return is_alpha(c) || is_digit(c);
    }
}    // namespace sleaf
