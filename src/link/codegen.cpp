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
    flushPrint();
    std::cout << "DONE: IR Generation took " << seconds << " seconds\n";
}

llvm::Value* CodeGenerator::generateExpressionIR(ASTNode* expressionNode) {
    // Placeholder implementation for expression generation
    return nullptr;
}

llvm::Function* CodeGenerator::generateFunctionDeclarationIR(ASTNode* functionNode) {
    std::vector<llvm::Type*> argTypes;

    // Collect argument types
    for (ASTNode* parameter : functionNode->children[0]->children) {

        // Check if nullptr
        if (parameter == nullptr) {
            if (debugMode) {
                std::cerr << "ERROR: Parameter is nullptr\n";
            }
            continue;
        }


        // Print the type of parameter
        std::cout << "Visiting " << parameter->type << "\n";
        
        // Assuming all parameters are of type `i32`
        argTypes.push_back(llvm::Type::getInt32Ty(context));

        flushPrint();
    }

    debugPrint("Creating function type\n");
    // Create function type
    llvm::FunctionType* functionType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context), argTypes, false);

    debugPrint("Creating function\n");
    // Create the function
    llvm::Function* function = llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, functionNode->lexeme, &module);


    debugPrint("Creating entry block\n");
    // Create a new basic block for the function entry
    llvm::BasicBlock* entryBlock =
        llvm::BasicBlock::Create(context, "entry", function);


    debugPrint("Creating builder\n");
    // Set the insert point to the entry block
    builder.SetInsertPoint(entryBlock);

    debugPrint("Creating function arguments\n");
    // Generate IR code for the function body
    generatePrintStatementIR(functionNode->children[1]);

    // TODO: Add a return instruction that doesnt seg fault
    // as the LL file is invalid without it.
    
    return function;
}

llvm::Value* CodeGenerator::generatePrintStatementIR(ASTNode* printNode) {
    std::string printValue = printNode->children[0]->lexeme;

    debugPrint("Creating print statement\n");
    // Create a constant string with the print value
    llvm::Value* stringValue = createStringConstant(printValue);

    debugPrint("Creating printf function call\n");

    // This RINT method is still incorrect and causes lld to error

    // Get the function type for rint
    llvm::FunctionType* rintType = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), { llvm::Type::getDoubleTy(context) }, false);

    // Create the rint function declaration
    llvm::Function* rintFunc = llvm::Function::Create(rintType, llvm::Function::ExternalLinkage, "rint", &module);

    debugPrint("Creating call instruction\n");
    builder.CreateCall(rintFunc, {stringValue});

    return nullptr;
}

llvm::Value* CodeGenerator::createStringConstant(const std::string& value) {
    llvm::Constant* stringConstant =
        llvm::ConstantDataArray::getString(context, value, true);

    llvm::GlobalVariable* globalString = new llvm::GlobalVariable(
        module, stringConstant->getType(), true,
        llvm::GlobalValue::PrivateLinkage, stringConstant);

    llvm::Constant* zero =
        llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(context));
    llvm::Constant* indices[] = {zero, zero};
    llvm::Constant* stringPtr =
        llvm::ConstantExpr::getInBoundsGetElementPtr(globalString->getValueType(),
                                                     globalString, indices);

    return stringPtr;
}