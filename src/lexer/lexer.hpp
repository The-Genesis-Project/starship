#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "token.hpp"

// Lexical analysis
std::vector<Token> lex(const std::string& sourceCode);
void printTokens(const std::vector<Token>& tokens);

// Utility functions
std::string tokenTypeToString(TokenType type);

#endif // LEXER_HPP

