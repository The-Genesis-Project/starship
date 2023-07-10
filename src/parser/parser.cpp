#include <iostream>
#include <thread>
#include <chrono>
#include <stack>

#include "parser.hpp"

#include "../util/globals.hpp"

// Global Lists
std::vector<std::unique_ptr<VariableBase>> variables;

// Global Functions

// Helper functions

// Check if a string is in regulation with the variable naming rules
bool validStringName(std::string& name) {
if (name.size() > 0) {
        if (name[0] >= '0' && name[0] <= '9') {
            return false;
        } else {
            for (char c : name) {
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || c == '_')) {
                    return false;
                }
            }
        }
    } else {
        return false;
    }

    return true;
}

bool isTypeToken(TokenType type) {
    return type == TokenType::INT || type == TokenType::STRING || type == TokenType::FLOAT;
}

bool isOperator(TokenType type) {
    return type == TokenType::PLUS || type == TokenType::MINUS || type == TokenType::STAR || type == TokenType::SLASH;
}

// Check if the operator on top of the stack has higher precedence than the current operator
bool hasHigherPrecedence(TokenType op1, TokenType op2) {
    if (op1 == TokenType::STAR || op1 == TokenType::SLASH) {
        return true;
    } else if (op1 == TokenType::PLUS || op1 == TokenType::MINUS) {
        if (op2 == TokenType::STAR || op2 == TokenType::SLASH) {
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

// Perform an operation on two operands
Token performOperation(const Token& left_operand, const Token& right_operand, const Token& operator_token) {
   // Use braced initialization to avoid "narrowing conversion" warnings

    if (operator_token.type == TokenType::PLUS) {
        return {TokenType::INT, std::to_string(std::stoi(left_operand.lexeme) + std::stoi(right_operand.lexeme)), 0};
    } else if (operator_token.type == TokenType::MINUS) {
        return {TokenType::INT, std::to_string(std::stoi(left_operand.lexeme) - std::stoi(right_operand.lexeme)), 0};
    } else if (operator_token.type == TokenType::STAR) {
        return {TokenType::INT, std::to_string(std::stoi(left_operand.lexeme) * std::stoi(right_operand.lexeme)), 0};
    } else if (operator_token.type == TokenType::SLASH) {
        return {TokenType::INT, std::to_string(std::stoi(left_operand.lexeme) / std::stoi(right_operand.lexeme)), 0};
    } else {
        return {TokenType::INT, "0", 0};
    }
}

// An actual expression parser
Token calculateExpression(const std::vector<Token>& expression_tokens) {
    std::stack<Token> operator_stack;
    std::stack<Token> operand_stack;

    for (const Token& token : expression_tokens) {
        if (token.type == TokenType::INT) {
            operand_stack.push(token);
        } else if (token.type == TokenType::PLUS || token.type == TokenType::MINUS ||
                   token.type == TokenType::STAR || token.type == TokenType::SLASH) {
            while (!operator_stack.empty() && hasHigherPrecedence(operator_stack.top().type, token.type)) {
                Token operator_token = operator_stack.top();
                operator_stack.pop();

                Token right_operand = operand_stack.top();
                operand_stack.pop();

                Token left_operand = operand_stack.top();
                operand_stack.pop();

                Token result = performOperation(left_operand, right_operand, operator_token);
                operand_stack.push(result);
            }

            operator_stack.push(token);
        }
    }

    while (!operator_stack.empty()) {
        Token operator_token = operator_stack.top();
        operator_stack.pop();

        Token right_operand = operand_stack.top();
        operand_stack.pop();

        Token left_operand = operand_stack.top();
        operand_stack.pop();

        Token result = performOperation(left_operand, right_operand, operator_token);
        operand_stack.push(result);
    }

    return operand_stack.top();
}

ASTNode* parseEquation(const std::vector<Token>& tokens, int& current) {
    auto* node = new ASTNode();
    node->type = "Equation";
    node->line = tokens[current].position;

    // Variable Name
    std::string variable_name = tokens[current].lexeme;

    // Read the type of variable this will be. (It's the previous token)
    TokenType variable_type = tokens[current - 1].type;

    current += 2; // Eat the IDENTIFIER and EQUAL tokens

    // Calculate the value of the expression
    std::vector<Token> expression_tokens;
    while (tokens[current].type != TokenType::SEMICOLON) {
        expression_tokens.push_back(tokens[current]);
        ++current;
    }

    Token result = calculateExpression(expression_tokens);
    // Check if the variable type matches the result type
    if (variable_type != result.type) {
        std::cerr << "Type mismatch for variable " << variable_name << " on line " << tokens[current].position << "\n";
        exit(1);
    }

    // Create and add the variable to the list.
    if (variable_type == TokenType::INT) {
        std::unique_ptr<Variable<int>> variable = std::make_unique<Variable<int>>();
        variable->name = variable_name;
        variable->value = std::stoi(result.lexeme);
        variable->type = TokenType::INT;
        variables.push_back(std::move(variable));
    } else {
        // Impossible, there must be an error
        std::cerr << "Unknown variable type\n";
        exit(1);
    }

    // Consume the semicolon
    ++current;

    return node;
}

// Update existing variables
ASTNode* updateVariable(const std::vector<Token>& tokens, int& current) {
    auto* node = new ASTNode();
    node->type = "Equation";
    node->line = tokens[current].position;

    // Variable Name
    std::string variable_name = tokens[current].lexeme;

    // Check if the variable exists
    bool found = false;

    // TODO: Later add virtual constructor to make the variables more generic
    VariableBase* varPointer = nullptr;

    for (const auto& variable : variables) {
        if (variable->name == tokens[current].lexeme) {
            found = true;
            // Get pointer
            varPointer = variable.get();
            break;
        }
    }

    if (!found) {
        std::cerr << "\033[1;31m" << "Variable " << variable_name << " on line " << tokens[current].position << " does not exist\n" << "\033[0m";
        exit(1);
    }

    current += 2; // Eat the IDENTIFIER and EQUAL tokens

    // Read the value of the expression
    // It might be a literal or a variable
    std::vector<Token> expression_tokens;
    while (tokens[current].type != TokenType::SEMICOLON) {
        expression_tokens.push_back(tokens[current]);
        ++current;
    }

    Token result = calculateExpression(expression_tokens);

    // Check if the variable type matches the result type
    if (varPointer->type != result.type) {
        std::cerr << "\033[1;31m" << "Type mismatch for variable " << variable_name << " on line " << tokens[current].position << "\n" << "\033[0m";
        exit(1);
    }

    // Update the variable
    dynamic_cast<Variable<int>*>(varPointer)->value = std::stoi(result.lexeme);

    return nullptr;
}

ASTNode* parseParameters(const std::vector<Token>& tokens, int& current, ASTNode* functionNode) {
    // Parameters node
    auto* node = new ASTNode();
    node->type = "Parameters";
    node->lexeme = "";
    node->line = tokens[current].position;

    // Consume the left parenthesis
    ++current;

    // Parse the identifiers inside the parentheses
    while (tokens[current].type != TokenType::RIGHT_PAREN) {
        // Parse the identifier
        auto* identifierNode = new ASTNode();
        identifierNode->type = "Variable";
        identifierNode->lexeme = tokens[current].lexeme;
        identifierNode->line = tokens[current].position;

        // Check if the identifier is valid
        if (tokens[current].type != TokenType::IDENTIFIER) {
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
            auto* typeNode = new ASTNode();
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

    // For now, we assume that the return statement is followed by one expression
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
    auto* expression = new ASTNode();
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
    auto* node = new ASTNode();
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
            // TODO: Very ugly, fix later...
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
        // Something is wrong if we get here
        std::cerr << "Unexpected '('\n";
        exit(1);
    }

    // RIGHT_PAREN Token
    if (tokens[current].type == TokenType::RIGHT_PAREN) {
        // Something is wrong if we get here
        std::cerr << "Unexpected ')'\n";
        exit(1);
    }

    // LEFT_BRACE Token
    if (tokens[current].type == TokenType::LEFT_BRACE) {
        // Something is wrong if we get here
        std::cerr << "Unexpected '{'\n";
        exit(1);
    }

    // RIGHT_BRACE Token
    if (tokens[current].type == TokenType::RIGHT_BRACE) {
        // Something is wrong if we get here
        std::cerr << "Unexpected '}'\n";
        exit(1);
    }

    // COMMA Token
    if (tokens[current].type == TokenType::COMMA) {
        // Something is wrong if we get here
        std::cerr << "Unexpected ','\n";
        exit(1);
    }

    // SEMICOLON Token
    if (tokens[current].type == TokenType::SEMICOLON) {
        ++current;
        return nullptr;
    }

    // FN Token
    if (tokens[current].type == TokenType::FN) {
        auto* node = new ASTNode();
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
        auto* node = new ASTNode();
        node->type = "PrintStatement";
        node->lexeme = "";
        node->line = tokens[current].position;
        ++current;

        // Parse the contents of the print statement
        std::vector<Token> printContents;
        while (tokens[current].type != TokenType::SEMICOLON) {
            printContents.push_back(tokens[current++]);
        }

        // Remove the first and last tokens as they are ( and )
        printContents.erase(printContents.begin());
        printContents.pop_back();

        // Check and ground variables.
        for (auto& token : printContents) {
            // Check if the token is an operator
            if (isOperator(token.type)) {
                continue;
            }

            // Check if the token is a number or string literal
            if (token.type == TokenType::INT || token.type == TokenType::STRING) {
                continue;
            }

            // Check if the token is a valid variable name
            std::string variable_name = token.lexeme;
            if (validStringName(variable_name)) {
                bool variable_found = false;

                // Check if it's in the variable list
                for (auto& list_variable : variables) {
                    if (variable_name == list_variable->name) {
                        variable_found = true;
                        list_variable->used = true;

                        // Replace the IDENTIFIER token with the correct node type for the variable
                        token.type = list_variable->type;
                        token.lexeme = std::to_string(dynamic_cast<Variable<int>*>(list_variable.get())->value);

                        break;
                    }
                }

                if (!variable_found) {
                    // Variable not found
                    std::cerr << "Variable not found: " << variable_name << " Line: " << token.position << "\n";
                }
            }
        }

        // Debug print the print contents
        std::cout << "Print contents: ";
        for (auto& token : printContents) {
            std::cout << "(" << token.lexeme << ", " << tokenTypeToString(token.type) << ")";
        }
        std::cout << "\n";

        // Calculate the result of the print statement
        Token printCalculationResult = calculateExpression(printContents);

        auto* printContentsNode = new ASTNode();
        printContentsNode->type = tokenTypeToString(printCalculationResult.type);
        printContentsNode->lexeme = printCalculationResult.lexeme;
        printContentsNode->line = node->line;

        node->children.push_back(printContentsNode);

        return node;
    }

    // FLOAT Token
    if (tokens[current].type == TokenType::FLOAT) {
        current++;
        return nullptr;
    }

    // INT Token
    if (tokens[current].type == TokenType::INT) {
        current++;
        return nullptr;
    }

    // STRING Token
    if (tokens[current].type == TokenType::STRING) {
        current++;
        return nullptr;
    }

    // END_OF_FILE Token
    if (tokens[current].type == TokenType::END_OF_FILE) {
        current++;
        return nullptr;
    }

    // RETURN Token
    if (tokens[current].type == TokenType::RETURN) {
        return parseReturn(tokens, current);
    }

    // IDENTIFIER Token
    if (tokens[current].type == TokenType::IDENTIFIER) {
        // Check if the previous token is a type token. This would mean that this is a variable declaration
        if(isTypeToken(tokens[current - 1].type)) {
            return parseEquation(tokens, current);
        }

        // If there is something else before the identifier, then it's a "variable update"
        if(!isTypeToken(tokens[current - 1].type)) {
            return updateVariable(tokens, current);
        }

        return nullptr;
    }

    // EQUAL Token
    if (tokens[current].type == TokenType::EQUAL) {
        ++current;
        return nullptr;
    }

    // If we don't recognize the token, return nullptr
    std::cerr << "Unrecognized token type: " << tokenTypeToString(tokens[current].type) << "\n at line: " << tokens[current].position << "\n";
    exit(1);
}

ASTNode* performParserAnalysis(const std::vector<Token>& tokens) {
    std::cout << "RUNNING: Starting Parser Analysis\n";

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

    auto* root = new ASTNode();
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
    flushPrint();
    std::cout << "DONE: Parser Analysis took " << seconds << " seconds\n";

    // Warn about unused variables
    for (auto& variable : variables) {
        if (!variable->used) {
            std::cerr << "\033[1;31m" << "WARNING: Unused variable: " << variable->name << "\033[0m\n"; // Red
        }
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

void flushPrint() {
    if (!verboseMode) {
        // Flush the output
        std::cout << "\033[1A\033[2K" << std::flush;
    }
}
