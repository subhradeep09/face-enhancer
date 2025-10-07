@echo off
echo === Building Face Enhancement C++ Server ===

REM Create build directory
if not exist build mkdir build
cd build

REM Configure CMake
echo Configuring CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed!
    echo Please ensure:
    echo 1. CMake is installed and in PATH
    echo 2. OpenCV is installed and can be found by CMake
    echo 3. Visual Studio Build Tools are installed
    pause
    exit /b 1
)

REM Build project
echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo === Build Complete ===
echo Executable: build\bin\FaceEnhancer.exe
echo.
echo To run the server:
echo cd build\bin
echo .\FaceEnhancer.exe
echo.
pause