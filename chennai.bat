@echo off
REM ============================================================================
REM chennai.bat — Chennai Lang Compiler Wrapper
REM
REM Usage:
REM   chennai <file.ch>                    Compile, assemble, link, and run
REM   chennai --asm <file.ch>              Generate .asm file only
REM   chennai --interp <file.ch>           Interpret (legacy mode)
REM   chennai --compile-only <file.ch>     Compile but don't run
REM ============================================================================

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "COMPILER=%SCRIPT_DIR%build\chennai_lang.exe"
set "JAVA_CP=%SCRIPT_DIR%build\java"

REM Check if compiler is built
if not exist "%COMPILER%" (
    echo [ERROR] Chennai Lang compiler not found at: %COMPILER%
    echo         Run build.bat first to build the compiler.
    exit /b 1
)

REM Parse arguments
set "MODE="
set "INPUT="
set "OUTPUT="
set "RUN_AFTER=1"

:parse_args
if "%~1"=="" goto done_args
if "%~1"=="--asm" (
    set "MODE=--asm-only"
    set "RUN_AFTER=0"
    shift
    goto parse_args
)
if "%~1"=="--interp" (
    set "MODE=--interpret"
    set "RUN_AFTER=0"
    shift
    goto parse_args
)
if "%~1"=="--compile-only" (
    set "RUN_AFTER=0"
    shift
    goto parse_args
)
if "%~1"=="-o" (
    set "OUTPUT=%~2"
    shift
    shift
    goto parse_args
)
set "INPUT=%~1"
shift
goto parse_args
:done_args

if "%INPUT%"=="" (
    echo Chennai Lang Compiler v2.0
    echo.
    echo Usage:
    echo   chennai ^<file.ch^>                  Compile and run
    echo   chennai --asm ^<file.ch^>            Generate .asm only
    echo   chennai --interp ^<file.ch^>         Interpret ^(legacy^)
    echo   chennai --compile-only ^<file.ch^>   Compile without running
    echo   chennai ^<file.ch^> -o ^<name^>      Custom output name
    exit /b 1
)

REM Get base name for output files
for %%F in ("%INPUT%") do set "BASENAME=%%~nF"

REM ── Interpret mode ─────────────────────────────────────────────────────────
if "%MODE%"=="--interpret" (
    echo [Chennai Lang] Interpreting: %INPUT%
    echo ────────────────────────────────────────
    "%COMPILER%" --interpret "%INPUT%"
    exit /b %ERRORLEVEL%
)

REM ── ASM-only mode ──────────────────────────────────────────────────────────
if "%MODE%"=="--asm-only" (
    echo [Chennai Lang] Generating assembly: %INPUT%
    "%COMPILER%" --asm-only "%INPUT%"
    exit /b %ERRORLEVEL%
)

REM ── Full compilation pipeline ──────────────────────────────────────────────
echo ════════════════════════════════════════════════
echo  Chennai Lang Compiler v2.0
echo ════════════════════════════════════════════════
echo.

REM Step 1: Java Lexer (try first)
set "TOKENS_FILE=%BASENAME%_tokens.json"
set "USED_JAVA=0"

where java >NUL 2>NUL
if %ERRORLEVEL% equ 0 (
    if exist "%JAVA_CP%\com\chennai\lexer\ChennaiLexer.class" (
        echo [Step 1] Lexing with Java Lexer...
        java -cp "%JAVA_CP%" com.chennai.lexer.ChennaiLexer "%INPUT%" > "%TOKENS_FILE%" 2>NUL
        if !ERRORLEVEL! equ 0 (
            set "USED_JAVA=1"
            echo          [OK] Tokens written to: %TOKENS_FILE%
        ) else (
            echo          [WARN] Java lexer failed, falling back to C++ lexer.
        )
    ) else (
        echo [Step 1] Java lexer not compiled, using C++ lexer.
    )
) else (
    echo [Step 1] Java not found, using C++ lexer.
)

REM Step 2: Parse + CodeGen + Assemble + Link
if "%USED_JAVA%"=="1" (
    echo [Step 2] Compiling with C++ backend (from Java tokens)...
    if "%OUTPUT%"=="" (
        "%COMPILER%" --tokens "%TOKENS_FILE%"
    ) else (
        "%COMPILER%" --tokens "%TOKENS_FILE%" -o "%OUTPUT%"
    )
) else (
    echo [Step 2] Compiling with C++ (lexer + parser + codegen)...
    if "%OUTPUT%"=="" (
        "%COMPILER%" --compile "%INPUT%"
    ) else (
        "%COMPILER%" --compile "%INPUT%" -o "%OUTPUT%"
    )
)

if %ERRORLEVEL% neq 0 (
    echo.
    echo [FAILED] Compilation failed!
    exit /b 1
)

REM Step 3: Run the compiled program
if "%RUN_AFTER%"=="1" (
    if "%OUTPUT%"=="" (
        set "EXEFILE=%BASENAME%.exe"
    ) else (
        set "EXEFILE=%OUTPUT%.exe"
    )
    
    echo.
    echo ════════════════════════════════════════════════
    echo  Running: !EXEFILE!
    echo ════════════════════════════════════════════════
    echo.
    "!EXEFILE!"
    echo.
    echo ────────────────────────────────────────
    echo  Exit code: !ERRORLEVEL!
)

endlocal
