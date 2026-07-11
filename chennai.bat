@echo off
REM chennai.bat — Chennai Lang Compiler Wrapper
set "PATH=C:\Users\nirle\Downloads\nasm-2.16.03-win64\nasm-2.16.03;C:\Users\nirle\Downloads\w64devkit\bin;%PATH%"
REM
REM Usage:
REM   chennai <file.ch>                    Compile, assemble, link, and run
REM   chennai --asm <file.ch>              Generate .asm file only
REM   chennai --compile-only <file.ch>     Compile but don't run

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
    echo   chennai --compile-only ^<file.ch^>   Compile without running
    echo   chennai ^<file.ch^> -o ^<name^>      Custom output name
    exit /b 1
)

REM Get base name for output files
for %%F in ("%INPUT%") do set "BASENAME=%%~nF"

REM ASM-only mode
if "%MODE%"=="--asm-only" (
    echo [Chennai Lang] Generating assembly: %INPUT%
    "%COMPILER%" --asm-only "%INPUT%"
    exit /b %ERRORLEVEL%
)

REM Full compilation pipeline
echo Chennai Lang Compiler v2.0
echo.

REM Compile directly using the C++ compiler which internally invokes the Java Lexer
if "%OUTPUT%"=="" (
    "%COMPILER%" "%INPUT%"
) else (
    "%COMPILER%" "%INPUT%" -o "%OUTPUT%"
)

if %ERRORLEVEL% neq 0 (
    echo.
    echo [FAILED] Compilation failed!
    exit /b 1
)

REM Run the compiled program
if "%RUN_AFTER%"=="1" (
    if "%OUTPUT%"=="" (
        set "EXEFILE=%BASENAME%.exe"
    ) else (
        set "EXEFILE=%OUTPUT%.exe"
    )
    
    echo.
    echo Running: !EXEFILE!
    echo.
    "!EXEFILE!"
    echo.
    echo Exit code: !ERRORLEVEL!
)

endlocal
