@echo off
REM ============================================================================
REM Chennai Lang — Build Script
REM Builds both the Java Lexer and the C++ Compiler
REM ============================================================================

echo ================================================
echo  Chennai Lang Compiler — Build System
echo ================================================
echo.

REM --------------------------------------------------------------------------
REM Step 1: Build Java Lexer
REM --------------------------------------------------------------------------
echo [1/2] Building Java Lexer...

where javac >NUL 2>NUL
if %ERRORLEVEL% neq 0 (
    echo [WARN] javac not found on PATH. Skipping Java Lexer build.
    echo        The compiler will use the C++ lexer as fallback.
    goto cpp_build
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

REM --------------------------------------------------------------------------
REM Step 2: Build C++ Compiler
REM --------------------------------------------------------------------------
:cpp_build
echo [2/2] Building C++ Compiler...

if not exist "build" mkdir "build"

pushd build
cmake .. -G "MinGW Makefiles" 2>NUL || cmake .. 2>NUL
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed!
    echo        Make sure CMake and a C++ compiler are installed.
    popd
    exit /b 1
)

cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo [ERROR] C++ build failed!
    popd
    exit /b 1
)
popd

echo.
echo ================================================
echo  Build Complete!
echo ================================================
echo.
echo  C++ Compiler:  build\chennai_lang.exe
echo  Java Lexer:    build\java\com\chennai\lexer\
echo.
echo  Usage:
echo    chennai.bat ^<file.ch^>         Compile and run
echo    chennai.bat --asm ^<file.ch^>   Generate ASM only
echo    chennai.bat --interp ^<file.ch^> Interpret (legacy)
echo.
