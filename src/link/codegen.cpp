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

void CodeGenerator::generateIR(ASTNode* rootNode) {
    std::cout << "RUNNING: Starting IR Generation\n";

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // Add function pass manager
    // llvm::legacy::FunctionPassManager functionPassManager(&module);

    // Add target data
    // TODO: Fix up the FPM 
    // functionPassManager.add(llvm::createTargetTransformInfoWrapperPass(llvm::TargetIRAnalysis()));

    // Visit the root node and generate IR code
    for (ASTNode* child : rootNode->children) {

        // Print the type of child
        std::cout << "Visiting " << child->type << "\n";

        if (child->type == "FunctionDeclaration") {
            generateFunctionDeclarationIR(child);
        }

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

llvm::Value* CodeGenerator::generateExpressionIR(ASTNode* expressionNode) {
    // Placeholder implementation for expression generation
    return nullptr;
}

llvm::Function* CodeGenerator::generateFunctionDeclarationIR(ASTNode* functionNode) {
    std::vector<llvm::Type*> argTypes;

    // Last child is the return type
    std::string functionReturnType = functionNode->children[0]->children.back()->lexeme;

    // Collect argument types
    // For all children except the last one
    debugPrint("Visiting " + std::to_string(functionNode->children[0]->children.size() - 1) + " parameters\n");

    for (int i = 0; i < functionNode->children[0]->children.size() - 1; i++) {
        ASTNode* parameter = functionNode->children[0]->children[i];

        debugPrint("Visiting parameter " + parameter->lexeme + "\n");

        // Parse out the type of the parameter
        std::string parameterType = parameter->children[0]->lexeme;
        
        llvm::Type* llvmType;

        if (parameterType == "int") {
            llvmType = llvm::Type::getInt32Ty(context);
        } else if (parameterType == "string") {
            llvmType = llvm::Type::getInt8PtrTy(context);
        } else {
            std::cerr << "ERROR: Invalid parameter type " << parameterType << "\n";
            continue;
        }

        // Add the type to the list of argument types
        argTypes.push_back(llvmType);

        if (!debugMode)
        {
            flushPrint();
        }

        // Debugprint "done visiting parameter"
        debugPrint("Done visiting parameter" + parameter->lexeme + "\n");
    }

    debugPrint("Creating function type\n");
    // Create function type
    // Switch through the return type to the llvm return type
    llvm::Type* llvmReturnType;

    // Print the return type
    debugPrint("Return type: " + functionReturnType + "\n");

    if (functionReturnType == "INT") {
        llvmReturnType = llvm::Type::getInt32Ty(context);
    } else if (functionReturnType == "STRING") {
        // Parse the return size of the string to correctly get the type
        std::cout << "String size: " << functionNode->children[1]->children.back()->children[0]->lexeme << "\n";
        int stringSize = functionNode->children[1]->children.back()->children[0]->lexeme.length() + 1;

        llvmReturnType = llvm::Type::getInt8Ty(context);
        llvmReturnType = llvm::ArrayType::get(llvmReturnType, stringSize);
        llvmReturnType = llvm::PointerType::get(llvmReturnType, 0);
    } else {
        std::cerr << "ERROR: Invalid return type \"" << functionReturnType << "\"\n";
        exit(1);
    }

    llvm::FunctionType* functionType = llvm::FunctionType::get(llvmReturnType, argTypes, false);

    debugPrint("Creating function\n");
    // Create the function
    llvm::Function* function = llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, functionNode->lexeme, &module);

    debugPrint("Creating entry block\n");
    // Create a new basic block for the function entry
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "", function);


    debugPrint("Creating builder\n");
    // Set the insert point to the entry block
    builder.SetInsertPoint(entryBlock);

    // Create the function contents
    // Loop through the statements and generate IR for each
    if (functionNode->children.size() >= 2 && functionNode->children[1] != nullptr) {
        for (ASTNode* statement : functionNode->children[1]->children) {
            if (statement == nullptr) {
                if (debugMode) {
                    std::cerr << "WARNING: Statement is nullptr\n";
                }
                continue;
            }

            // Debug print the statement type and the function name
            debugPrint("Visiting " + statement->type + " in function " + functionNode->lexeme + "\n");

            if (statement->type == "PrintStatement") {
                debugPrint("Generating print statement IR\n");
                generatePrintStatementIR(statement);
            }
        }
    }
   
    debugPrint("Creating return instruction for function \"" + functionNode->lexeme + "\"\n");

    // I don't know why this is needed, but it is, and it fixes the seg fault 
    // TODO: Figure out why and remove it
   llvm::IRBuilder<> builder(entryBlock);

    // As before, switch through the return type to the llvm return type
    llvm::Type* llvmFunctionReturnType;
    std::string functionBottomReturnType = functionNode->children.back()->children.back()->children[0]->type;

    auto returnValue = functionNode->children.back()->children.back()->children[0]->lexeme;
    llvm::Value* returnPointer;

    // Create a switch statement to handle different return types
    if (functionBottomReturnType == "INT") {
        returnPointer = llvm::ConstantInt::get(context, llvm::APInt(32, std::stoi(returnValue), true));
    } else if (functionBottomReturnType == "STRING") {
        llvm::Value* stringConstant = createStringConstant(returnValue);
        returnPointer = stringConstant;
    } else {
        // Handle other return types or error case
        std::cerr << "Unsupported return type: " << functionBottomReturnType << "\n";
        exit(1);
    }

    // Create the return instruction
    builder.CreateRet(returnPointer);

    // Verify the function
    debugPrint("Verifying function\n");
    llvm::verifyFunction(*function);
    
    return function;
}

llvm::Value* CodeGenerator::generatePrintStatementIR(ASTNode* printNode) {
    std::string printValue = printNode->children[0]->lexeme;

    debugPrint("Creating print statement\n");

    // handle different types of \ characters
    std::string processedValue;
    std::size_t i = 0;
    while (i < printValue.length()) {
        char c = printValue[i++];
        if (c == '\\') {
            if (i < printValue.length()) {
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
