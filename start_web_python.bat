@echo off
echo ================================================
echo    Face Enhancer - Web Interface (Python)
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

REM Navigate to web directory
cd /d "%~dp0web"

echo Starting Face Enhancer Web Interface...
echo.
echo The web interface will open in your default browser
echo Access URL: http://localhost:8080
echo.
echo Features:
echo   - Upload images via drag ^& drop or click
echo   - Adjust enhancement parameters with sliders
echo   - Real-time before/after comparison
echo   - Download enhanced images
echo   - Quality metrics display
echo.
echo Press Ctrl+C to stop the server
echo ================================================

REM Start the Python web server
python python_server.py

pause