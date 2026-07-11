cmake_minimum_required(VERSION 3.16)
project(chennai_lang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Chennai Lang Compiler (Java + C++ Hybrid)
# ============================================================================

add_executable(chennai_lang
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
    src/codegen.cpp
    src/token_reader.cpp
)

target_include_directories(chennai_lang PRIVATE src)

# ============================================================================
# Custom target: Build Java Lexer
# ============================================================================

# Find Java compiler
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
    message(WARNING "javac not found — Java Lexer will not be built. "
                    "The compiler will use the C++ lexer as fallback.")
endif()

# ============================================================================
# Installation
# ============================================================================

install(TARGETS chennai_lang DESTINATION bin)
