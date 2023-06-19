cmake_minimum_required(VERSION 3.0)
project(starship)

set(SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
)

add_executable(starship ${SOURCES})

