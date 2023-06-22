#include <iostream>
#include <thread>
#include <chrono>

#include "parser.hpp"

#include "../util/globals.hpp"

ASTNode* parseExpression(const std::vector<Token>& tokens, int& current) {
    ASTNode* node = new ASTNode();
    node->type = "String";
    node->line = tokens[current].position;

    // Consume the LEFT_PAREN token
    ++current;

    // Print the value of the expression
    std::cout << "Expression value: " << tokens[current].lexeme << "\n";
    node->lexeme = tokens[current].lexeme;

    // Consume the expression and the RIGHT_PAREN token
    ++current;
    ++current;

    flushPrint();
    return node;
}

ASTNode* parseParameters(const std::vector<Token>& tokens, int& current, ASTNode* functionNode) {
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

        // Add the identifier as a child of the parameters node
        node->children.push_back(identifierNode);

        // Consume the identifier
        ++current;

        // If the next token is a colon, parse the type
        if (tokens[current].type == TokenType::COLON) {
            // Consume the colon
            ++current;

            // Parse the type
            ASTNode* typeNode = new ASTNode();
            typeNode->type = "Type";
            typeNode->lexeme = tokens[current].lexeme;
            typeNode->line = tokens[current].position;

            // Add the type as a child of the identifier node
            identifierNode->children.push_back(typeNode);

            // Consume the type
            ++current;
        } else {
            // Give error with information on what function and what variable name
            std::cerr << "Expected colon after variable " << identifierNode->lexeme << " in function " << functionNode->lexeme << "\n";
            exit(1);
        }

        // If the next token is a comma, consume it
        if (tokens[current].type == TokenType::COMMA) {
            ++current;
        }
    }

    // Consume the right parenthesis
    ++current;

    // Check if the next token is an arrow (return type indicator)
    if (tokens[current].type == TokenType::ARROW) {
        ++current;

        // Parse the return type
        std::string returnType = tokens[current].lexeme;

        // Create the Return node and add the return type as a child
        ASTNode* returnNode = new ASTNode();
        // Use the return node Type to indicate the return type
        returnNode->type = "Return";
        returnNode->lexeme = tokenTypeToString(tokens[current].type);
        returnNode->line = tokens[current].position;

        // Add the Return node as a child of the Parameters node
        node->children.push_back(returnNode);
    }

    ++current;

    return node;
}


ASTNode* parseReturn(const std::vector<Token>& tokens, int& current) {
    // Return node
    ASTNode* node = new ASTNode();
    node->type = "Return";
    node->line = tokens[current].position;

    // For now we assume that the return statement is followed by one expression
    // Parse the expression and add it as a child of the return node
    // Not using parseExpression() because of custom paren handling

    // Consume the return token
    if (tokens[current].type == TokenType::RETURN) {
        ++current;
    } else {
        std::cerr << "Expected return\n";
        exit(1);
    }


    // Parse the expression
    ASTNode* expression = new ASTNode();
    expression->type = tokenTypeToString(tokens[current].type);
    expression->lexeme = tokens[current].lexeme;
    expression->line = tokens[current].position;

    // Consume the expression
    ++current;

    // Add the expression as a child of the return node
    node->children.push_back(expression);

    // Consume the semicolon
    if (tokens[current].type == TokenType::SEMICOLON) {
        ++current;
    } else {
        std::cerr << "Expected semicolon\n";
        exit(1);
    }

    return node;
}

ASTNode* parseFunctionBody(const std::vector<Token>& tokens, int& current, ASTNode* functionNode) {
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
        // Expection keywords
        if (tokens[current].type == TokenType::RETURN) {
            // Parse the return statement
            ASTNode* returnNode = parseReturn(tokens, current);

            // Check that the return statement is the last statement in the function body
            if (tokens[current].type != TokenType::RIGHT_BRACE) {
                std::cerr << "Return statement must be last statement in function body\n";
                exit(1);
            }

            // Check that the return statment type matches the function return type
            // Very ugly, fix later...
            if (returnNode->children[0]->type != functionNode->children[0]->children.back()->lexeme) {
                std::cerr << "Return type does not match function return type\n";

                std::cerr << "\n";
                printAST(functionNode, 0);
                std::cerr << "\n";

                std::cerr << "Return type: " << returnNode->children[0]->type << "\n";
                std::cerr << "Function return type " << functionNode->children[0]->children.back()->lexeme << "\n";
                exit(1);
            }

            node->children.push_back(returnNode);
            continue;
        }

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

    flushPrint();

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
        node->children.push_back(parseParameters(tokens, current, node));

        // Parse the function body
        node->children.push_back(parseFunctionBody(tokens, current, node));

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

    // RETURN Token
    if (tokens[current].type == TokenType::RETURN) {
        return parseReturn(tokens, current);
    }

    // If we don't recognize the token, return nullptr
    std::cerr << "Unrecognized token type: " << tokenTypeToString(tokens[current].type) << "\n at line: " << tokens[current].position << "\n";
    return nullptr;
}

ASTNode* performParserAnalysis(const std::vector<Token>& tokens) {
    std::cout << "RUNNING: Starting Parser Analysis\n";

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

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

    // Stop the timer
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

    // Print the elapsed time
    double seconds = duration / 1e9;  // Convert nanoseconds to seconds
    flushPrint();
    std::cout << "DONE: Parser Analysis took " << seconds << " seconds\n";

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

void flushPrint() {
    if (!verboseMode) {
        // Flush the output
        std::cout << "\033[1A\033[2K" << std::flush;
    }
}
