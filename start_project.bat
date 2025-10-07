@echo off
title Face Enhancement Project Launcher

echo ========================================
echo    Face Enhancement Project Launcher
echo ========================================
echo.
echo Select which version to run:
echo.
echo [1] Python Version (Ready to use)
echo [2] C++ Version (Requires CMake + Visual Studio)
echo [3] Exit
echo.

set /p choice="Enter your choice (1-3): "

if "%choice%"=="1" (
    echo.
    echo Starting Python Face Enhancement Server...
    cd web
    call start_server.bat
) else if "%choice%"=="2" (
    echo.
    echo Starting C++ Face Enhancement Server...
    cd cpp
    call start_server.bat
) else if "%choice%"=="3" (
    echo Goodbye!
    exit /b 0
) else (
    echo Invalid choice. Please select 1, 2, or 3.
    pause
    goto start
)

:start
pause