#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "token.hpp"

std::vector<Token> lex(const std::string& sourceCode);
void printTokens(const std::vector<Token>& tokens);

#endif // LEXER_HPP

