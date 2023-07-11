#include <chrono>
#include <iostream>
#include "codegen.hpp"

void debugPrint(const std::string& message) {
    if (debugMode) {
        std::cout << "[DEBUG] " << message;
    }
}


CodeGenerator::CodeGenerator(llvm::Module& module)
    : module(module), context(module.getContext()), builder(context) {}

void CodeGenerator::generateIR(ASTTree* rootNode) {
    std::cout << "RUNNING: Starting IR Generation\n";

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // Add function pass manager
    // llvm::legacy::FunctionPassManager functionPassManager(&module);

    // Add target data
    // TODO: Fix up the FPM 
    // functionPassManager.add(llvm::createTargetTransformInfoWrapperPass(llvm::TargetIRAnalysis()));

    // Visit the root node and generate IR code
    for (ASTNodeBase* child : rootNode->statements) {

        // Print the type of child
        std::cout << "Visiting " << typeid(*child).name() << "\n";

        /*
        if (child->type == "FunctionDeclaration") {
            generateFunctionDeclarationIR(child);
        }
         */

        flushPrint();
    }

    std::error_code errorCode;
    llvm::raw_fd_ostream outputFile("output.ll", errorCode, llvm::sys::fs::OpenFlags::OF_None);

    // Check if the file was opened successfully
    if (errorCode) {
        std::cerr << "Failed to open output file: " << errorCode.message() << "\n";
        return;
    }

    // Set the output stream for printing the module
    module.print(outputFile, nullptr);


     // Stop the timer
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

    // Print the elapsed time
    double seconds = duration / 1e9;  // Convert nanoseconds to seconds
    std::cout << "DONE: IR Generation took " << seconds << " seconds\n";
}

llvm::Function* CodeGenerator::generateFunctionDeclarationIR(FunctionNode* functionNode) {
    std::vector<llvm::Type*> argTypes;

    // Last child is the return type
    TokenType functionReturnType = functionNode->returnVariable.type;

    // Collect argument types
    // TODO: I don't like the double parameter thing.
    debugPrint("Visiting " + std::to_string(functionNode->parameters.parameters.size()) + " parameters\n");

    // Goes through each parameter.
    for (int i = 0; i < functionNode->parameters.parameters.size() - 1; i++) {
        VariableBase* parameter = functionNode->parameters.parameters[i];

        debugPrint("Visiting parameter " + parameter->name + "\n");

        // Parse out the type of the parameter
        TokenType parameterType = parameter->type;
        
        llvm::Type* llvmType;

        if (parameterType == TokenType::INT) {
            llvmType = llvm::Type::getInt32Ty(context);
        } else if (parameterType == TokenType::STRING) {
            llvmType = llvm::Type::getInt8PtrTy(context);
        } else {
            std::cerr << "ERROR: Invalid parameter type " << tokenTypeToString(parameterType) << "\n";
            continue;
        }

        // Add the type to the list of argument types
        argTypes.push_back(llvmType);

        if (!debugMode)
        {
            flushPrint();
        }

        // Debug print "done visiting parameter"
        debugPrint("Done visiting parameter" + parameter->name + "\n");
    }

    debugPrint("Creating function type\n");

    // Create function type
    // Switch through the return type to the llvm return type
    llvm::Type* llvmReturnType;

    // Print the return type
    debugPrint("Return type: " + tokenTypeToString(functionReturnType) + "\n");

    if (functionReturnType == TokenType::INT) {
        llvmReturnType = llvm::Type::getInt32Ty(context);
    } /* else if (functionReturnType == TokenType::STRING) {
        // Parse the return size of the string to correctly get the type
        std::cout << "String size: " << functionNode->children[1]->children.back()->children[0]->lexeme << "\n";
        int stringSize = functionNode->children[1]->children.back()->children[0]->lexeme.length() + 1;

        llvmReturnType = llvm::Type::getInt8Ty(context);
        llvmReturnType = llvm::ArrayType::get(llvmReturnType, stringSize);
        llvmReturnType = llvm::PointerType::get(llvmReturnType, 0);
    } */ else {
        std::cerr << "ERROR: Invalid return type \"" << tokenTypeToString(functionReturnType) << "\"\n";
        exit(1);
    }

    llvm::FunctionType* functionType = llvm::FunctionType::get(llvmReturnType, argTypes, false);

    debugPrint("Creating function\n");
    // Create the function
    llvm::Function* function = llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, functionNode->name, &module);

    debugPrint("Creating entry block\n");
    // Create a new basic block for the function entry
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "", function);


    debugPrint("Creating builder\n");
    // Set the insert point to the entry block
    builder.SetInsertPoint(entryBlock);

    // Create the function contents
    // Loop through the statements and generate IR for each
    if (functionNode->body->statements.size() >= 2 && functionNode->body->statements[1] != nullptr) {
        for (ASTNodeBase* statement : functionNode->body->statements) {
            if (statement == nullptr) {
                if (debugMode) {
                    std::cerr << "WARNING: Statement is nullptr\n";
                }
                continue;
            }

            // Debug print the statement type and the function name
            debugPrint("Visiting " + std::string(typeid(*statement).name()) + " in function " + functionNode->name + "\n");

            /*
            if (statement->type == "PrintStatement") {
                debugPrint("Generating print statement IR\n");
                generatePrintStatementIR(statement);
            }
                */
        }
    }
   
    debugPrint("Creating return instruction for function \"" + functionNode->name + "\"\n");

    // I don't know why this is needed, but it is, and it fixes the seg fault 
    // TODO: Figure out why and remove it
   llvm::IRBuilder<> builder(entryBlock);

    // As before, switch through the return type to the llvm return type
    llvm::Type* llvmFunctionReturnType;
    TokenType functionBottomReturnType = functionNode->returnVariable.type;

    VariableBase returnVariable = functionNode->returnVariable;
    llvm::Value* returnPointer;

    // Create a switch statement to handle different return types
    if (functionBottomReturnType == TokenType::INT) {
        Variable<int>* intReturnVariable = dynamic_cast<Variable<int>*>(&returnVariable);
        if (intReturnVariable != nullptr) {
            int value = intReturnVariable->value;
            returnPointer = llvm::ConstantInt::get(context, llvm::APInt(32, value, true));
        }
    }
    /*else if (functionBottomReturnType == TokenType::STRING) {
        llvm::Value* stringConstant = createStringConstant(returnValue);
        returnPointer = stringConstant;
    } */
    else {
        // Handle other return types or error case
        std::cerr << "Unsupported return type: " << tokenTypeToString(functionBottomReturnType) << "\n";
        exit(1);
    }

    // Create the return instruction
    builder.CreateRet(returnPointer);

    // Verify the function
    debugPrint("Verifying function\n");
    llvm::verifyFunction(*function);
    
    return function;
}

llvm::Value* CodeGenerator::generatePrintStatementIR(PrintNode* printNode) {
    VariableBase* printValue = printNode->expression;
    std::string processedValue;

    // Switch through the different types the print value could be and dynamically cast it
    switch (printValue->type) {
        case TokenType::INT: {
            processedValue = std::to_string(dynamic_cast<Variable<int>*>(printValue)->value);
        }
        // Add more later.
    }

    // Check if processedValue is empty
    if (processedValue.empty()) {
        std::cerr << "ERROR: Invalid print value\n";
        exit(1);
    }

    debugPrint("Creating print statement\n");

    /* This will be used in the string case
    // handle different types of \ characters
    std::string processedValue;
    std::size_t i = 0;
    while (i < printValue) {
        char c = printValue[i++];
        if (c == '\\') {
            if (i < printValue.) {
                char nextChar = printValue[i++];
                switch (nextChar) {
                    case 'n': processedValue += '\n'; break;
                    case 't': processedValue += '\t'; break;
                    case '\\': processedValue += '\\'; break;
                    default:
                        processedValue += c;
                        processedValue += nextChar;
                        break;
                }
            } else {
                processedValue += c;
            }
        } else {
            // Regular character
            processedValue += c;
        }
    }
    */

    llvm::Value* stringValue = createStringConstant(processedValue);

    debugPrint("Creating printf function call\n");
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::IntegerType::getInt32Ty(context),
        {llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0)},
        true
    );

    llvm::FunctionCallee printCallee = module.getOrInsertFunction("printf", printfType);

    debugPrint("Creating call instruction\n");

    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    llvm::Value* indices[] = {zero, zero};

    llvm::Value* stringPointer = builder.CreateGEP(
        llvm::cast<llvm::PointerType>(stringValue->getType()->getScalarType())->getElementType(),
        stringValue,
        indices
    );

    builder.CreateCall(printCallee, {stringPointer});

    return nullptr;
}

llvm::Value* CodeGenerator::createStringConstant(const std::string& value) {
    llvm::IRBuilder<> builder(context);
    builder.SetInsertPoint(&module.getFunctionList().front().getEntryBlock(), module.getFunctionList().front().getEntryBlock().begin());
    llvm::Value *valStr = builder.CreateGlobalString(value);

    return valStr;
}
