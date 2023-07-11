#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include <utility>

#include "../parser/parser.hpp"
#include "../util/globals.hpp"

class CodeGenerator {
public:
    CodeGenerator(llvm::Module& module);
    void generateIR(ASTTree* rootNode);

private:
    llvm::Function* generateFunctionDeclarationIR(FunctionNode* functionNode);
    llvm::Value* generatePrintStatementIR(PrintNode* printNode);

    llvm::Value* createStringConstant(const std::string& value);

    llvm::Module& module;
    llvm::IRBuilder<> builder;
    llvm::LLVMContext& context;
};

#endif  // CODEGEN_HPP
