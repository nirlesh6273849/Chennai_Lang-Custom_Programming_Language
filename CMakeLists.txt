cmake_minimum_required(VERSION 3.16)
project(chennai_lang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Chennai Lang Compiler (Java + C++ Hybrid)
add_executable(chennai_lang
    src/main.cpp
    src/parser.cpp
    src/codegen.cpp
    src/token_reader.cpp
)

target_include_directories(chennai_lang PRIVATE src)

# Custom target: Build Java Lexer
find_program(JAVAC javac)

if(JAVAC)
    message(STATUS "Found javac: ${JAVAC}")
    
    file(GLOB JAVA_SOURCES
        ${CMAKE_SOURCE_DIR}/src/java/com/chennai/lexer/*.java
    )
    
    add_custom_target(java_lexer
        COMMAND ${JAVAC} -d ${CMAKE_BINARY_DIR}/java
            ${CMAKE_SOURCE_DIR}/src/java/com/chennai/lexer/TokenType.java
            ${CMAKE_SOURCE_DIR}/src/java/com/chennai/lexer/Token.java
            ${CMAKE_SOURCE_DIR}/src/java/com/chennai/lexer/ChennaiLexer.java
        COMMENT "Compiling Java Lexer..."
        SOURCES ${JAVA_SOURCES}
    )

    # Make the main build depend on the Java lexer
    add_dependencies(chennai_lang java_lexer)
else()
    message(FATAL_ERROR "javac not found. The Java Lexer is strictly required to build the Chennai Lang compiler.")
endif()

# Installation
install(TARGETS chennai_lang DESTINATION bin)
