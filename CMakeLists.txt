cmake_minimum_required(VERSION 3.0)
project(starship)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/link/codegen.cpp
)

add_executable(starship ${SOURCES})

llvm_map_components_to_libnames(llvm_libs
  asmparser
  support
  core
  irreader
  executionengine
  interpreter
  mcjit
  orcjit
  x86codegen
  x86desc
  x86info
  x86utils
  native
)

target_link_libraries(starship ${LLVM_LIBS})
