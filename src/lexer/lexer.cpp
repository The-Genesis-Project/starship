#include <iostream>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"

std::vector<Token> lex(const std::string& sourceCode) {
    std::vector<Token> tokens;

    std::size_t position = 0;
    std::size_t line = 1;

    while (position < sourceCode.size()) {
        char currentChar = sourceCode[position];

        // Handle single-character tokens
        switch (currentChar) {
            case '(': tokens.emplace_back(TokenType::LEFT_PAREN, "(", line); break;
            case ')': tokens.emplace_back(TokenType::RIGHT_PAREN, ")", line); break;
            case '{': tokens.emplace_back(TokenType::LEFT_BRACE, "{", line); break;
            case '}': tokens.emplace_back(TokenType::RIGHT_BRACE, "}", line); break;
            case ',': tokens.emplace_back(TokenType::COMMA, ",", line); break;
            case '.': tokens.emplace_back(TokenType::DOT, ".", line); break;
            case ';': tokens.emplace_back(TokenType::SEMICOLON, ";", line); break;
        }

        // Handle whitespace
        if (currentChar == ' ' || currentChar == '\t' || currentChar == '\r') {
            position++;
            continue;
        }

        // Handle newlines
        if (currentChar == '\n') {
            line++;
            position++;
            continue;
        }

        // Handle identifiers and keywords
        if (isalpha(currentChar)) {
            std::size_t identifierStart = position;
            while (position < sourceCode.size() && isalnum(sourceCode[position])) {
                position++;
            }
            std::string lexeme = sourceCode.substr(identifierStart, position - identifierStart);
            TokenType type;
            if (lexeme == "fn") {
                type = TokenType::FN;
            } else if (lexeme == "const") {
                type = TokenType::CONST;
            } else if (lexeme == "let") {
                type = TokenType::LET;
            } else if (lexeme == "print") {
                type = TokenType::PRINT;
            } else {
                type = TokenType::IDENTIFIER;
            }
            tokens.emplace_back(type, lexeme, line);
            continue;
        }

        // Handle string literals
        if (currentChar == '\"') {
            std::size_t stringStart = position + 1;
            position++;
            while (position < sourceCode.size() && sourceCode[position] != '\"') {
                position++;
            }
            if (position == sourceCode.size()) {
                std::cerr << "Unterminated string literal at line " << line << "\n";
                break;
            }
            std::string lexeme = sourceCode.substr(stringStart, position - stringStart);
            tokens.emplace_back(TokenType::STRING, lexeme, line);
            position++;
            continue;
        }

        // Handle unrecognized characters
        std::cerr << "Unrecognized character '" << currentChar << "' at line " << line << "\n";
        break;
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", line);
    return tokens;
}

std::ostream& operator<<(std::ostream& os, const TokenType& tokenType) {
    switch (tokenType) {
        case TokenType::LEFT_PAREN:
            os << "LEFT_PAREN";
            break;
        case TokenType::RIGHT_PAREN:
            os << "RIGHT_PAREN";
            break;
        case TokenType::LEFT_BRACE:
            os << "LEFT_BRACE";
            break;
        case TokenType::RIGHT_BRACE:
            os << "RIGHT_BRACE";
            break;
        case TokenType::COMMA:
            os << "COMMA";
            break;
        case TokenType::DOT:
            os << "DOT";
            break;
        case TokenType::SEMICOLON:
            os << "SEMICOLON";
            break;
        case TokenType::FN:
            os << "FN";
            break;
        case TokenType::CONST:
            os << "CONST";
            break;
        case TokenType::LET:
            os << "LET";
            break;
        case TokenType::PRINT:
            os << "PRINT";
            break;
        case TokenType::IDENTIFIER:
            os << "IDENTIFIER";
            break;
        case TokenType::STRING:
            os << "STRING";
            break;
        case TokenType::END_OF_FILE:
            os << "END_OF_FILE";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}


void printTokens(const std::vector<Token>& tokens) {
    for (const Token& token : tokens) {
        std::cout << "[" << token.type << ", \"" << token.lexeme << "\", " << token.position << "]\n";
    }
}

