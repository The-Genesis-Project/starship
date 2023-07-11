#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "token.hpp"

// Lexical analysis
std::vector<Token> lex(const std::string& sourceCode);
std::vector<Token> performLexicalAnalysis(const std::string& sourceCode);

// Utility functions
std::string tokenTypeToString(TokenType type);
TokenType stringToTokenType(std::string toke_string);
void printTokens(const std::vector<Token>& tokens);

#endif // LEXER_HPP

