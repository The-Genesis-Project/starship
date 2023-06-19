#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <vector>

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, SEMICOLON,

    // Keywords
    FN, CONST, LET, PRINT,

    // Literals
    IDENTIFIER, STRING,

    // End-of-file
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t position;

    Token(TokenType type, const std::string& lexeme, std::size_t position)
        : type(type), lexeme(lexeme), position(position) {}
};

using TokenStream = std::vector<Token>;

#endif // TOKEN_HPP

