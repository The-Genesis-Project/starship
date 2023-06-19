#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <chrono>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "link/codegen.hpp"

#include "util/globals.hpp"

bool debugMode = false;
bool verboseMode = false;

void printUsage() {
    std::cout << "Usage: starship [options] <input-file>\n";
    std::cout << "Options:\n";
    std::cout << "  --version   Print version information\n";
    std::cout << "  build       Builds the project (main.rk)\n";
    std::cout << "    Build Flags:\n";
    std::cout << "    -d / --debug     Prints extra debug messages\n";
    std::cout << "    -v / --verbose   Prints extra build messages\n";
    std::cout << "    Example usage, either -dv or -d -v, both work\n";
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

    if (inputFilename == "build") {

        // Check for build flags
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-d" || arg == "--debug") {
                debugMode = true;
            } else if (arg == "-v" || arg == "--verbose") {
                verboseMode = true;
            } else if (arg.substr(0, 2) == "-d") {
                // Handle joined debug flags
                debugMode = true;
                arg = arg.substr(2); // Remove the "-d" part
                for (char c : arg) {
                    if (c == 'v') {
                        verboseMode = true;
                    } else {
                        std::cerr << "Error: Unknown build flag: " << c << "\n";
                        std::exit(1);
                    }
                }
            } else if (arg.substr(0, 2) == "-v") {
                // Handle joined verbose flags
                verboseMode = true;
                arg = arg.substr(2); // Remove the "-v" part
                for (char c : arg) {
                    if (c == 'd') {
                        debugMode = true;
                    } else {
                        std::cerr << "Error: Unknown build flag: " << c << "\n";
                        std::exit(1);
                    }
                }
            } else {
                std::cerr << "Error: Unknown build flag: " << arg << "\n";
                std::exit(1);
            }
        }


        // Build tool
        std::string sourceFilename = "main.rk";
        std::string outputFilename = "main";

        std::ifstream sourceFile(sourceFilename);

        if (!sourceFile) {
            std::cerr << "Failed to open input file: " << sourceFilename << "\n";
            return 1;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        std::string sourceCode((std::istreambuf_iterator<char>(sourceFile)),
                               std::istreambuf_iterator<char>());

        
        // Lexical analysis
        std::vector<Token> tokens = performLexicalAnalysis(sourceCode);

        // Parsing analysis
        ASTNode* ast = performParserAnalysis(tokens);

        if (debugMode) {
            std::cout << "\n";
            printAST(ast, 0);
        }

        // Clean up
        delete ast;

        std::cout << "\nSuccessfully built: " << outputFilename << "\n";

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

        double seconds = duration / 1e9;  // Convert nanoseconds to seconds
        std::cout << "Building took " << seconds << " seconds\n";

        return 0;
    }

    // Normal execution
    std::ifstream inputFile(inputFilename);
    if (!inputFile) {
        std::cerr << "Failed to open input file: " << inputFilename << "\n";
        return 1;
    }

    return 0;
}

