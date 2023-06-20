#include "globals.hpp"
#include <iostream>

void debugPrint(const std::string& message) {
    if (debugMode) {
        std::cout << "[DEBUG] " << message << "\n";
    }
}