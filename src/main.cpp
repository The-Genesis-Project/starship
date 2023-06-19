#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "link/codegen.hpp"

void printUsage() {
    std::cout << "Usage: starship [options] <input-file>\n";
    std::cout << "Options:\n";
    std::cout << "  --version   Print version information\n";
}

void printVersion() {
    std::cout << "Starship Compiler v0.1\n";
    std::cout << "Made by David Rubin <daviru007@icloud.com>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string inputFilename = argv[1];

    if (inputFilename == "--version") {
        printVersion();
        return 0;
    }

    std::ifstream inputFile(inputFilename);
    if (!inputFile) {
        std::cerr << "Failed to open input file: " << inputFilename << "\n";
        return 1;
    }

    std::string sourceCode((std::istreambuf_iterator<char>(inputFile)),
                           std::istreambuf_iterator<char>());

    std::cout << "Source code:\n\n";
    printf("%s", sourceCode.c_str());
    std::cout << "\n\n";

    // Lexical analysis
    std::vector<Token> tokens = lex(sourceCode);

    // Print tokens
    std::cout << "Tokens:\n\n";
    printTokens(tokens);
    std::cout << "\n\n";

    // Parsing analysis
    ASTNode* ast = parse(tokens);

    std::cout << "\nAST:\n";
    printAST(ast, 0);

    // Start LLVM IR Generation


    // Free memory
    delete ast;

    return 0;
}

