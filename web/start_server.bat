@echo off
title Face Enhancement Server - Python

echo ========================================
echo    Face Enhancement Server (Python)
echo ========================================
echo.

REM Check if we're in the correct directory
if not exist "enhanced_server.py" (
    echo ERROR: enhanced_server.py not found!
    echo Please run this script from the web directory.
    echo Current directory: %CD%
    pause
    exit /b 1
)

REM Check if Python is available
python --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: Python is not installed or not in PATH!
    echo Please install Python and try again.
    pause
    exit /b 1
)

REM Check if OpenCV is available
echo Checking OpenCV installation...
python -c "import cv2; print('✅ OpenCV version:', cv2.__version__)" 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: OpenCV is not installed!
    echo Please install OpenCV: pip install opencv-python
    pause
    exit /b 1
)

echo.
echo ========================================
echo Starting Face Enhancement Server...
echo ========================================
echo.
echo 🚀 Server will start on: http://localhost:8083
echo 🌐 Browser will open automatically
echo 🛑 Press Ctrl+C to stop the server
echo.

REM Create a batch file to open browser after delay
echo @echo off > open_browser.bat
echo timeout /t 3 /nobreak ^>nul >> open_browser.bat
echo start http://localhost:8083 >> open_browser.bat
echo del open_browser.bat >> open_browser.bat

REM Start browser opener in background
start /min open_browser.bat

REM Start the enhanced server (this will also try to open browser)
python enhanced_server.py

echo.
echo ========================================
echo         Server Stopped
echo ========================================
pause