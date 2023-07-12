#include <iostream>
#include <thread>
#include <chrono>
#include <stack>

#include "parser.hpp"

#include "../util/globals.hpp"

// Global Lists
std::vector<std::unique_ptr<VariableBase>> variables;

// Global Functions
std::vector<FunctionNode> functions;

// Helper functions

// Check if a string is in regulation with the variable naming rules
bool validStringName(std::string& name) {
if (!name.empty()) {
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

    if (expression_tokens.size() == 1) {
        return expression_tokens[0];
    }

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

VariableBase parseEquation(const std::vector<Token>& tokens, int& current) {

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


    // Consume the semicolon
    ++current;

    // Create and add the variable to the list.
    if (variable_type == TokenType::INT) {
        std::unique_ptr<Variable<int>> variable = std::make_unique<Variable<int>>();
        variable->name = variable_name;
        variable->value = std::stoi(result.lexeme);
        variable->type = TokenType::INT;
        variables.push_back(std::move(variable));

        return *variables.back();
    } else {
        // Impossible, there must be an error
        std::cerr << "Unknown variable type\n";
        exit(1);
    }
}

// Update existing variables
void updateVariable(const std::vector<Token>& tokens, int& current) {

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
}

ParameterNode* parseParameters(const std::vector<Token>& tokens, int& current) {
    // Parameters node
    auto* node = new ParameterNode();

    // Consume the left parenthesis
    ++current;

    // Parse the identifiers inside the parentheses
    while (tokens[current].type != TokenType::RIGHT_PAREN) {

        if (tokens[current].type != TokenType::IDENTIFIER) {
            std::cerr << "Expected variable identifier\n";
            exit(1);
        }

        // Consume the identifier
        ++current;

        // Declare the variable
        VariableBase* variable = new VariableBase();
        variable->name = tokens[current - 1].lexeme;

        // If the next token is a colon, parse the type
        if (tokens[current].type == TokenType::COLON) {
            // Consume the colon
            ++current;

            // Parse the type
            TokenType type = tokens[current].type;

            // Create the variable
            if (type == TokenType::INT) {
                variable = new Variable<int>();
                variable->type = TokenType::INT;

                node->parameters.push_back(variable);
            } else {
                std::cerr << "Unknown or unsupported type of variable\n";
                exit(1);
            }

            // Consume the type
            ++current;
        } else {
            std::cerr << "Expected colon after variable identifier for: " << variable->name << "\n";
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
        TokenType returnType = stringToTokenType(tokens[current].lexeme);

        node->returnType = returnType;

        // Consume the return type
        ++current;
    } else {
        std::cerr << "Function has no return type.\n";
        exit(1);
    }

    return node;
}

VariableBase* parseReturn(const std::vector<Token>& tokens, int& current) {

    VariableBase* variable;

    // Consume the return token
    if (tokens[current].type == TokenType::RETURN) {
        ++current;
    } else {
        std::cerr << "Expected function return.\n";
        exit(1);
    }

    // Parse the expression
    std::vector<Token> expression_tokens;
    while (tokens[current].type != TokenType::SEMICOLON) {
        expression_tokens.push_back(tokens[current]);
        ++current;
    }

    Token result = calculateExpression(expression_tokens);

    // Debug print the result of the expression
    std::cout << "Result of expression: " << result.lexeme << "\n";

    // Set the value of the variable
    if (result.type == TokenType::INT) {
        // Set the variable to be an <int>
        variable = new Variable<int>();
        dynamic_cast<Variable<int>*>(variable)->value = std::stoi(result.lexeme); // Improve this. Shouldn't need to dynamic cast
        dynamic_cast<Variable<int>*>(variable)->type = result.type; // More secure than just setting TokenType::INT
    } else {
        std::cerr << "Unknown or unsupported type of variable\n";
        exit(1);
    }

    // Consume the semicolon
    if (tokens[current].type == TokenType::SEMICOLON) {
        ++current;
    } else {
        std::cerr << "Expected semicolon\n";
        exit(1);
    }

    return variable;
}

FunctionBodyNode* parseFunctionBody(const std::vector<Token>& tokens, int& current, FunctionNode* functionNode) {
    // Code to parse function body goes here
    // This will use recursive descent parsing to parse the statements inside the function body

    // Function body node
    auto* node = new FunctionBodyNode();

    // Consume the left brace
    if (tokens[current].type == TokenType::LEFT_BRACE) {
        ++current;
    } else {
        std::cerr << "Expected left brace\n";
        exit(1);
    }

    // Parse the statements inside the function body
    while (tokens[current].type != TokenType::RIGHT_BRACE) {
        // Exception keywords
        if (tokens[current].type == TokenType::RETURN) {
            // Parse the return statement
            VariableBase* returnNode = parseReturn(tokens, current);

            functionNode->returnVariable = *returnNode;
            // For some reason the functionNode->returnVariable is drifting, so this sets it back to the correct value
            // TODO: Fix function return variable drifting

            // Check that the return statement is the last statement in the function body
            if (tokens[current].type != TokenType::RIGHT_BRACE) {
                std::cerr << "Return statement must be last statement in function body\n";
                exit(1);
            }

            // Check that the return statement type matches the function return type
            // TODO: Very ugly, fix later...
            if (returnNode->type != functionNode->returnVariable.type) {
                std::cerr << "Return type does not match function return type\n";

                std::cerr << "\n";
                printAST(functionNode, 0);
                std::cerr << "\n";

                std::cerr << "Return type: " << tokenTypeToString(returnNode->type) << "\n";
                std::cerr << "Function return type " << tokenTypeToString(functionNode->returnVariable.type) << "\n";
                exit(1);
            }

            functionNode->returnVariable.type = returnNode->type;
            continue;
        }

        // Parse each statement and add it as a child of the function body node
        ASTNodeBase* statement = parseStatement(tokens, current);
        node->statements.push_back(statement);
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

ASTNodeBase* parseStatement(const std::vector<Token>& tokens, int& current) {
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
        auto* node = new FunctionNode();

        // Consume the FN token
        ++current;

        // Parse the function name
        node->name = tokens[current].lexeme;
        ++current;

        // Parse the function parameters and print them
        // Why is there a * here?
        node->parameters = *parseParameters(tokens, current);

        // Parse the function body
        node->body = parseFunctionBody(tokens, current, node);

        // Debug Print the name, return type, and parameters of the function
        std::cout << "Function name: " << node->name << "\n";
        std::cout << "Function return type: " << tokenTypeToString(node->returnVariable.type) << "\n";
        std::cout << "Function parameters: " << "\n";
        for (auto& parameter : node->parameters.parameters) {
            std::cout << "Parameter name: " << parameter->name << "\n";
            std::cout << "Parameter type: " << tokenTypeToString(parameter->type) << "\n";
        }

        return node;
    }

    // PRINT Token
    if (tokens[current].type == TokenType::PRINT) {
        auto* node = new PrintNode();

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

        switch(printCalculationResult.type) {
            case TokenType::INT:
                Variable<int> printResult;
                printResult.type = TokenType::INT;
                printResult.value = std::stoi(printCalculationResult.lexeme);
                node->expression = &printResult;
            // Add more types later.
        }

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

    // IDENTIFIER Token
    if (tokens[current].type == TokenType::IDENTIFIER) {
        // Check if the previous token is a type token. This would mean that this is a variable declaration
        if(isTypeToken(tokens[current - 1].type)) {
            auto* variable = new VariableNode();
            variable->variable = parseEquation(tokens, current);
        }

        // If there is something else before the identifier, then it's a "variable update"
        if(!isTypeToken(tokens[current - 1].type)) {
            updateVariable(tokens, current);
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

ASTTree* performParserAnalysis(const std::vector<Token>& tokens) {
    std::cout << "RUNNING: Starting Parser Analysis\n";

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

    auto* root = new ASTTree();

    int current = 0;
    while (current < tokens.size()) {
        ASTNodeBase* statement = parseStatement(tokens, current);
        if (statement != nullptr) {
            root->statements.push_back(statement);
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

void printAST(ASTNodeBase* node, int indent) {
    if (node == nullptr) {
        return;
    }

    std::string indentation(indent, ' ');

    // Unicode tree lines
    std::string verticalLine = u8"\u251C\u2500";    // ├─
    std::string cornerLine = u8"\u2514\u2500";     // └─
    std::string branchLine = u8"\u2502 ";          // │

    std::cout << indentation;

    // Print the node type
    std::cout << std::string(typeid(*node).name()) << "\n";
}

void flushPrint() {
    if (!verboseMode) {
        // Flush the output
        std::cout << "\033[1A\033[2K" << std::flush;
    }
}
