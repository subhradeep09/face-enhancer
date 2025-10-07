@echo off
title Face Enhancement C++ Server

echo ========================================
echo    Face Enhancement C++ Server
echo ========================================
echo.

REM Check if already built
if not exist "build\bin\FaceEnhancer.exe" (
    echo Building the server for the first time...
    echo.
    call build.bat
    if %ERRORLEVEL% neq 0 (
        echo Build failed! Please check the errors above.
        pause
        exit /b 1
    )
    echo.
)

REM Start the server
echo Starting Face Enhancement Server...
echo Server will be available at: http://localhost:8084
echo Press Ctrl+C to stop the server
echo.

cd build\bin
FaceEnhancer.exe

echo.
echo Server stopped.
pause