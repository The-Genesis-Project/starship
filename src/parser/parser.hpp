#ifndef ASTGEN_HPP
#define ASTGEN_HPP

#include "../lexer/lexer.hpp"

struct ASTNode {
    std::string type;
    std::string lexeme;
    int line;
    std::vector<ASTNode*> children;
};

// Base class for variables
struct VariableBase {
    std::string name;
    TokenType type;
    bool used;
    virtual ~VariableBase() = default;
};

template <typename T>
struct Variable : public VariableBase {
    T value;
};

struct Function {
    std::string name;
    TokenType returnType;

};

ASTNode* performParserAnalysis(const std::vector<Token>& tokens);

ASTNode* parseStatement(const std::vector<Token>& tokens, int& current);
ASTNode* calculateExpression(const std::vector<Token>& tokens, int& current);
ASTNode* parseFunction(const std::vector<Token>& tokens, int& current);
ASTNode* parseParameters(const std::vector<Token>& tokens, int& current);

void printAST(const ASTNode* node, int indent);
void flushPrint();

#endif // ASTGEN_HPP

