cmake_minimum_required(VERSION 3.16)
project(chennai_lang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chennai_lang
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
)

target_include_directories(chennai_lang PRIVATE src)
