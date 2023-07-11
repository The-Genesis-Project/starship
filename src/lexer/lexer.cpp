#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "token.hpp"
#include "lexer.hpp"
#include "../parser/parser.hpp"

std::vector<Token> lex(const std::string& sourceCode) {
    std::vector<Token> tokens;

    std::size_t position = 0;
    std::size_t line = 1;

    while (position < sourceCode.size()) {
        char currentChar = sourceCode[position];

        // Handle single-character tokens
        switch (currentChar) {
            case '(':
                tokens.emplace_back(TokenType::LEFT_PAREN, "(", line);
                position++;
                continue;
            case ')':
                tokens.emplace_back(TokenType::RIGHT_PAREN, ")", line);
                position++;
                continue;
            case '{':
                tokens.emplace_back(TokenType::LEFT_BRACE, "{", line);
                position++;
                continue;
            case '}':
                tokens.emplace_back(TokenType::RIGHT_BRACE, "}", line);
                position++;
                continue;
            case ',':
                tokens.emplace_back(TokenType::COMMA, ",", line);
                position++;
                continue;
            case ';':
                tokens.emplace_back(TokenType::SEMICOLON, ";", line);
                position++;
                continue;
            case ':':
                tokens.emplace_back(TokenType::COLON, ":", line);
                position++;
                continue;
            case '=':
                tokens.emplace_back(TokenType::EQUAL, "=", line);
                position++;
                continue;
            case '+':
                tokens.emplace_back(TokenType::PLUS, "+", line);
                position++;
                continue;
            case '*':
                tokens.emplace_back(TokenType::STAR, "*", line);
                position++;
                continue;
            case '/':
                tokens.emplace_back(TokenType::SLASH, "/", line);
                position++;
                continue;
            case '@':
                tokens.emplace_back(TokenType::AT, "@", line);
                position++;
                continue;

            // Integers
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                std::size_t numericStart = position;
                while (position < sourceCode.size() && isdigit(sourceCode[position])) {
                    position++;
                }
                std::string lexeme = sourceCode.substr(numericStart, position - numericStart);
                tokens.emplace_back(TokenType::INT, lexeme, line);
                continue;
            }

            // Floats. Checks for an INT followed by a '.' followed by a digit
            case '.': {
                if (position + 1 < sourceCode.size() && isdigit(sourceCode[position + 1])) {
                    std::size_t numericStart = position;
                    position++;
                    while (position < sourceCode.size() && isdigit(sourceCode[position])) {
                        position++;
                    }
                    // Eat the past object because it's going to be the start "int" of a float
                    std::string lexeme = tokens.back().lexeme + sourceCode.substr(numericStart, position - numericStart);
                    tokens.pop_back();
                    tokens.emplace_back(TokenType::FLOAT, lexeme, line);
                    continue;
                }
            }
        }

        // Handle two-character tokens
        if (currentChar == '-' && position + 1 < sourceCode.size() && sourceCode[position + 1] == '>') {
            tokens.emplace_back(TokenType::ARROW, "->", line);
            position += 2;
            continue;
        }

        // Handle the - after the arrow token because of interference with the previous if statement
        // TODO: Find a better way to do this. Probably check if the previous token is a - and then if there's a >
        if (currentChar == '-') {
            tokens.emplace_back(TokenType::MINUS, "-", line);
            position++;
            continue;
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
            } else if (lexeme == "print") {
                type = TokenType::PRINT;
            } else if (lexeme == "return") {
                type = TokenType::RETURN;
            } else if (lexeme == "int") {
                type = TokenType::INT;
            } else if (lexeme == "string") {
                type = TokenType::STRING;
            } else if (lexeme == "float") {
                type = TokenType::FLOAT;
            } else if (lexeme == "import") {
                type = TokenType::IMPORT;
            }
            else {
                type = TokenType::IDENTIFIER;
            }
            tokens.emplace_back(type, lexeme, line);
            continue;
        }

        // Handle string literals
        // TODO: Handle character interferences like test_variable and such. The _ is preventing the string from being recognized
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
        exit(1);
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", line);
    return tokens;
}

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN:
            return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN:
            return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE:
            return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE:
            return "RIGHT_BRACE";
        case TokenType::COMMA:
            return "COMMA";
        case TokenType::SEMICOLON:
            return "SEMICOLON";
        case TokenType::FN:
            return "FN";
        case TokenType::PRINT:
            return "PRINT";
        case TokenType::ARROW:
            return "ARROW";
        case TokenType::COLON:
            return "COLON";

        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::EQUAL:
            return "EQUAL";

        case TokenType::INT:
            return "INT";
        case TokenType::FLOAT:
            return "FLOAT";
        case TokenType::STRING:
            return "STRING";

        case TokenType::PLUS:
            return "PLUS";
        case TokenType::MINUS:
            return "MINUS";
        case TokenType::STAR:
            return "STAR";
        case TokenType::SLASH:
            return "SLASH";

        case TokenType::END_OF_FILE:
            return "END_OF_FILE";
        case TokenType::RETURN:
            return "RETURN";

    }

    return "";
}

void printTokens(const std::vector<Token>& tokens) {
    for (const Token& token : tokens) {
        std::cout << tokenTypeToString(token.type) << " " << token.lexeme << " " << token.position << "\n";
    }
}

std::vector<Token> performLexicalAnalysis(const std::string& sourceCode) {
    std::cout << "RUNNING: Starting Lexical Analysis\n";

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // Perform lexical analysis
    std::vector<Token> tokens = lex(sourceCode);

    // Stop the timer
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

    // Print the elapsed time
    double seconds = duration / 1e9;  // Convert nanoseconds to seconds
    flushPrint();
    std::cout << "DONE: Lexical Analysis took " << seconds << " seconds\n";

    return tokens;
}

