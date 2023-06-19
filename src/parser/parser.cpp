#include <iostream>

#include "parser.hpp"

ASTNode* parseExpression(const std::vector<Token>& tokens, int& current) {
    ASTNode* node = new ASTNode();
    node->type = "String";
    node->line = tokens[current].position;

    // Consume the LEFT_PAREN token
    ++current;

    // Print the value of the expression
    std::cout << "Print value: " << tokens[current].lexeme << "\n";
    node->lexeme = tokens[current].lexeme;

    // Consume the expression and the RIGHT_PAREN token
    ++current;
    ++current;

    return node;
}

ASTNode* parseParameters(const std::vector<Token>& tokens, int& current) {
    // Code to parse parameters goes here
    // This will use recursive descent parsing to parse parameters

    // Parameters node
    ASTNode* node = new ASTNode();
    node->type = "Parameters";
    node->lexeme = "";
    node->line = tokens[current].position;

    // Consume the left parenthesis
    ++current;

    // Parse the identifiers inside the parentheses
    while (tokens[current].type != TokenType::RIGHT_PAREN) {
        // Parse the identifier
        ASTNode* identifierNode = new ASTNode();
        identifierNode->type = "Variable";
        identifierNode->lexeme = tokens[current].lexeme;
        identifierNode->line = tokens[current].position;

        // Check if the identifier is valid
        if (tokens[current].type != TokenType::FLOAT) {
            std::cerr << "Expected variable\n";
            exit(1);
        }

        // Check if the identifier name is made of only letters
        for (char c : tokens[current].lexeme) {
            if (!isalpha(c)) {
                std::cerr << "Identifier names can only have letters\n";
                exit(1);
            }
        }

        // Print the identifier lexeme
        std::cout << "Identifier Name: " << tokens[current].lexeme << "\n";

        // Add the identifier as a child of the parameters node
        node->children.push_back(identifierNode);

        // Consume the identifier
        ++current;

        // If the next token is a comma, consume it
        if (tokens[current].type == TokenType::COMMA) {
            ++current;
        }
    }    

    // Consume the right parenthesis
    ++current;

    return node;
}

ASTNode* parseFunctionBody(const std::vector<Token>& tokens, int& current) {
    // Code to parse function body goes here
    // This will use recursive descent parsing to parse the statements inside the function body

    // Function body node
    ASTNode* node = new ASTNode();
    node->type = "FunctionBody";
    node->line = tokens[current].position;

    // Consume the left brace
    if (tokens[current].type == TokenType::LEFT_BRACE) {
        ++current;
    } else {
        std::cerr << "Expected left brace\n";
        exit(1);
    }

    // Parse the statements inside the function body
    while (tokens[current].type != TokenType::RIGHT_BRACE) {
        // Parse each statement and add it as a child of the function body node
        ASTNode* statement = parseStatement(tokens, current);
        node->children.push_back(statement);
    }

    // Consume the right brace
    if (tokens[current].type == TokenType::RIGHT_BRACE) {
        ++current;
    } else {
        std::cerr << "Expected right brace\n";
        exit(1);
    }

    return node;
}

ASTNode* parseStatement(const std::vector<Token>& tokens, int& current) {
    // Print type of current token
    std::cout << "Current token type: " << tokenTypeToString(tokens[current].type) << "\n";

    // LEFT_PAREN Token
    if (tokens[current].type == TokenType::LEFT_PAREN) {
        ASTNode* node = new ASTNode();
        node->type = "LeftParen";
        node->lexeme = tokens[current].lexeme;
        node->line = tokens[current].position;
        ++current;

        // Parse the expression inside the parentheses
        node->children.push_back(parseExpression(tokens, current));

        // Expect a right parenthesis
        if (tokens[current].type != TokenType::RIGHT_PAREN) {
            std::cerr << "Expected ')' after expression\n";
            exit(1);
        }

        // Consume the right parenthesis
        ++current;
        // stop the program here for debug
        exit(1);
        return node;
    }

    // RIGHT_PAREN Token
    if (tokens[current].type == TokenType::RIGHT_PAREN) {
        // Something is wrong if we get here
        exit(1);
    }

    // LEFT_BRACE Token
    if (tokens[current].type == TokenType::LEFT_BRACE) {
        ASTNode* node = new ASTNode();
        node->type = "LeftBrace";
        node->lexeme = tokens[current].lexeme;
        node->line = tokens[current].position;
        ++current;

        // Parse the expression inside the braces
        node->children.push_back(parseExpression(tokens, current));

        // Expect a right brace
        if (tokens[current].type != TokenType::RIGHT_BRACE) {
            std::cerr << "Expected '}' after expression\n";
            exit(1);
        }

        // Consume the right brace
        ++current;

        return node;
    }

    // RIGHT_BRACE Token
    if (tokens[current].type == TokenType::RIGHT_BRACE) {
        // Something is wrong if we get here
        exit(1);
    }

    // COMMA Token
    if (tokens[current].type == TokenType::COMMA) {
        ASTNode* node = new ASTNode();
        node->type = "Comma";
        node->lexeme = tokens[current].lexeme;
        node->line = tokens[current].position;
        ++current;

        // Parse the expression after the comma
        node->children.push_back(parseExpression(tokens, current));

        return node;
    }

    // SEMICOLON Token
    if (tokens[current].type == TokenType::SEMICOLON) {
        ++current;
        return nullptr;
    }

    // FN Token
    if (tokens[current].type == TokenType::FN) {
        ASTNode* node = new ASTNode();
        node->type = "FunctionDeclaration";
        node->lexeme = tokens[++current].lexeme;
        node->line = tokens[current].position;

        ++current;

        // Parse the function parameters and print them
        node->children.push_back(parseParameters(tokens, current));

        // Parse the function body
        node->children.push_back(parseFunctionBody(tokens, current));

        return node;
    }

    // PRINT Token
    if (tokens[current].type == TokenType::PRINT) {
        ASTNode* node = new ASTNode();
        node->type = "PrintStatement";
        node->lexeme = "";
        node->line = tokens[current].position;
        ++current;

        // Parse the expression inside the print statement
        node->children.push_back(parseExpression(tokens, current));

        return node;
    }

    // IDENTIFIER Token
    if (tokens[current].type == TokenType::FLOAT) {
        ASTNode* node = new ASTNode();
        node->type = "Float";
        node->lexeme = tokens[current].lexeme;
        node->line = tokens[current].position;
        ++current;

        return node;
    }

    // STRING Token
    if (tokens[current].type == TokenType::STRING) {
        ASTNode* node = new ASTNode();
        node->type = "String";
        node->lexeme = tokens[current].lexeme;
        node->line = tokens[current].position;
        ++current;

        return node;
    }

    // END_OF_FILE Token
    if (tokens[current].type == TokenType::END_OF_FILE) {
        ++current;
        return nullptr;
    }

    // If we don't recognize the token, return nullptr
    std::cerr << "Unrecognized token type: " << tokenTypeToString(tokens[current].type) << "\n at line: " << tokens[current].position << "\n";
    return nullptr;
}

ASTNode* parse(const std::vector<Token>& tokens) {
    ASTNode* root = new ASTNode();
    root->type = "Program";
    root->line = 1;

    int current = 0;
    while (current < tokens.size()) {
        ASTNode* statement = parseStatement(tokens, current);
        if (statement != nullptr) {
            root->children.push_back(statement);
        }
        // If the statement is null, we just skip it
    }

    return root;
}

void printAST(const ASTNode* node, int indent) {
    if (node == nullptr) {
        return;
    }

    std::string indentation(indent, ' ');

    // Unicode tree lines
    std::string verticalLine = u8"\u251C\u2500";    // ├─
    std::string cornerLine = u8"\u2514\u2500";     // └─
    std::string branchLine = u8"\u2502 ";          // │

    std::cout << indentation;

    if (indent > 0) {
        std::cout << branchLine;
    }

    std::cout << node->type << " - " << node->lexeme << " (Line: " << node->line << ")\n";

    int numChildren = node->children.size();
    for (int i = 0; i < numChildren; ++i) {
        const ASTNode* child = node->children[i];
        bool isLastChild = (i == numChildren - 1);

        std::string childIndentation = indentation + (isLastChild ? "   " : cornerLine + "  ");

        printAST(child, indent + 4);
    }
}
