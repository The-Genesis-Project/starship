#ifndef ASTGEN_HPP
#define ASTGEN_HPP

#include "../lexer/lexer.hpp"

struct ASTNode {
    std::string type;
    std::string lexeme;
    int line;
    std::vector<ASTNode*> children;
};

ASTNode* parse(const std::vector<Token>& tokens);
ASTNode* parseStatement(const std::vector<Token>& tokens, int& current);
ASTNode* parseFunction(const std::vector<Token>& tokens, int& current);
ASTNode* parseExpression(const std::vector<Token>& tokens, int& current);
ASTNode* parseParameters(const std::vector<Token>& tokens, int& current);

void printAST(const ASTNode* node, int indent);

#endif // ASTGEN_HPP

