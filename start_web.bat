@echo off
echo ================================================
echo    Face Enhancer - Web Interface
echo ================================================
echo.

REM Check if Python is installed
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Python not found!
    echo.
    echo Please install Python from: https://python.org/
    echo Then run this script again.
    echo.
    pause
    exit /b 1
)

echo Python version:
python --version
echo.

echo Starting Face Enhancer Web Interface...
echo.
echo The web interface will open at: http://localhost:8000
echo Press Ctrl+C to stop the server
echo ================================================

REM Start the simple Python web server
cd /d "%~dp0web"
start http://localhost:8000/simple.html
python -m http.server 8000

pause