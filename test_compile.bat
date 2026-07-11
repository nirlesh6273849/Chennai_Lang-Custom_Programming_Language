@echo off
REM ============================================================================
REM test_compile.bat — Test all Chennai Lang examples
REM ============================================================================

setlocal enabledelayedexpansion

echo ════════════════════════════════════════════════════════
echo  Chennai Lang — Compilation Test Suite
echo ════════════════════════════════════════════════════════
echo.

set "PASS=0"
set "FAIL=0"
set "TOTAL=0"

for %%F in (examples\*.ch) do (
    set /a TOTAL+=1
    echo ──────────────────────────────────────────────────
    echo  Testing: %%F
    echo ──────────────────────────────────────────────────

    REM Try to compile (ASM only for testing codegen)
    call chennai.bat --asm "%%F"
    
    if !ERRORLEVEL! equ 0 (
        echo  [PASS] %%F compiled successfully
        set /a PASS+=1
    ) else (
        echo  [FAIL] %%F failed to compile
        set /a FAIL+=1
    )
    echo.
)

echo ════════════════════════════════════════════════════════
echo  Results: !PASS!/!TOTAL! passed, !FAIL! failed
echo ════════════════════════════════════════════════════════

if !FAIL! gtr 0 (
    exit /b 1
)
