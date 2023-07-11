#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <vector>

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    COMMA, SEMICOLON, COLON,
    AT,

    // Expression tokens
    EQUAL, // =
    IDENTIFIER, // Variable name
    PLUS, MINUS, STAR, SLASH, // + - * /

    // Two-character tokens
    ARROW, // ->

    // Keywords
    FN, PRINT, IMPORT,

    // Literals
    INT, // U64
    FLOAT, // F64
    STRING, // String

    // End-of-file
    END_OF_FILE,
    RETURN
};

struct Token {
    TokenType type;
    std::string lexeme; // This is basically the name.
    std::size_t position;

    Token(TokenType type, const std::string& lexeme, std::size_t position)
        : type(type), lexeme(lexeme), position(position) {}
};

using TokenStream = std::vector<Token>;

#endif // TOKEN_HPP

