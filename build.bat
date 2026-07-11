@echo off
REM Chennai Lang — Build Script
set "PATH=C:\Users\nirle\Downloads\nasm-2.16.03-win64\nasm-2.16.03;C:\Users\nirle\Downloads\w64devkit\bin;%PATH%"
REM Builds both the Java Lexer and the C++ Compiler

echo Building Chennai Lang Compiler

REM Step 1: Build Java Lexer
echo [1/2] Building Java Lexer...

where javac >NUL 2>NUL
if %ERRORLEVEL% neq 0 (
    echo [ERROR] javac not found on PATH. The Java Lexer is strictly required.
    exit /b 1
)

if not exist "build\java" mkdir "build\java"

javac -d build\java ^
    src\java\com\chennai\lexer\TokenType.java ^
    src\java\com\chennai\lexer\Token.java ^
    src\java\com\chennai\lexer\ChennaiLexer.java

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Java Lexer compilation failed!
    exit /b 1
)
echo [OK] Java Lexer built successfully.
echo.

REM Step 2: Build C++ Compiler
echo [2/2] Building C++ Compiler...

if not exist "build" mkdir "build"

g++ -O2 -std=c++17 -I src src\main.cpp src\parser.cpp src\codegen.cpp src\token_reader.cpp -o build\chennai_lang.exe
if %ERRORLEVEL% neq 0 (
    echo [ERROR] C++ build failed!
    exit /b 1
)

echo.
echo Build Complete!
echo.
echo C++ Compiler:  build\chennai_lang.exe
echo Java Lexer:    build\java\com\chennai\lexer\
echo.
echo Usage:
echo   chennai.bat ^<file.ch^>         Compile and run
echo   chennai.bat --asm ^<file.ch^>   Generate ASM only
echo.
