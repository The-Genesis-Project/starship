cmake_minimum_required(VERSION 3.0)
project(starship)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/link/codegen.cpp
)

add_executable(starship ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(starship ${llvm_libs})
