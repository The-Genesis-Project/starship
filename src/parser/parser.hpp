#ifndef ASTGEN_HPP
#define ASTGEN_HPP

#include "../lexer/lexer.hpp"

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

struct ASTNodeBase {
    virtual ~ASTNodeBase() {}
};

struct ParameterNode : public ASTNodeBase {
    std::vector<VariableBase*> parameters;
    TokenType returnType; // This is the return type declared in the parameter list.
};

// A wrapper for a variable node
struct VariableNode : public ASTNodeBase {
    VariableBase variable;
};

struct FunctionBodyNode : public ASTNodeBase {
    std::vector<ASTNodeBase*> statements;
};

struct PrintNode : public ASTNodeBase {
    VariableBase* expression;
    TokenType type; // Used for dynamically casting it later on. There probably is a better solution.
};

struct FunctionNode : public ASTNodeBase {
    std::string name;
    VariableBase* returnVariable; // This is the return type at the bottom of the function.
    ParameterNode parameters;
    FunctionBodyNode* body;
};

struct ASTTree {
    std::vector<ASTNodeBase*> statements;
};

ASTTree* performParserAnalysis(const std::vector<Token>& tokens);

ASTNodeBase* parseStatement(const std::vector<Token>& tokens, int& current);
ASTNodeBase* calculateExpression(const std::vector<Token>& tokens, int& current);
ASTNodeBase* parseFunction(const std::vector<Token>& tokens, int& current);
ParameterNode* parseParameters(const std::vector<Token>& tokens, int& current);

void printAST(ASTNodeBase* node, int indent);
void flushPrint();

#endif // ASTGEN_HPP

