@echo off
echo ====================================
echo   Face Enhancer - C++ Build Script
echo ====================================
echo.

REM Check if CMake is installed
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found. Please install CMake first.
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

echo Configuring project with CMake...
echo.

REM Try to configure with vcpkg first (if available)
if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    echo Using vcpkg toolchain...
    cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
) else if exist "D:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    echo Using vcpkg toolchain...
    cmake .. -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
) else (
    echo Using system OpenCV installation...
    cmake .. -DCMAKE_BUILD_TYPE=Release
)

if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Possible issues:
    echo 1. OpenCV not installed or not found
    echo 2. Missing C++ compiler (Visual Studio or MinGW)
    echo 3. Incorrect CMake configuration
    echo.
    echo Solutions:
    echo 1. Install OpenCV using vcpkg:
    echo    vcpkg install opencv4[contrib]:x64-windows
    echo 2. Install Visual Studio Community 2022 with C++ tools
    echo 3. Make sure OpenCV is in your PATH
    echo.
    pause
    exit /b 1
)

echo.
echo Building project...
echo.

cmake --build . --config Release --parallel

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    echo Check the error messages above for details.
    pause
    exit /b 1
)

echo.
echo ====================================
echo   Build completed successfully!
echo ====================================
echo.
echo Executable location: build\Release\face_enhancer.exe
echo.

REM Test if the executable was created
if exist "Release\face_enhancer.exe" (
    echo Testing the executable...
    Release\face_enhancer.exe --help 2>nul
    if %errorlevel% equ 0 (
        echo SUCCESS: Executable is working correctly!
    ) else (
        echo WARNING: Executable built but might have runtime issues
        echo Make sure OpenCV DLLs are in PATH
    )
) else if exist "face_enhancer.exe" (
    echo Testing the executable...
    face_enhancer.exe --help 2>nul
    if %errorlevel% equ 0 (
        echo SUCCESS: Executable is working correctly!
    ) else (
        echo WARNING: Executable built but might have runtime issues
        echo Make sure OpenCV DLLs are in PATH
    )
) else (
    echo WARNING: Executable not found in expected location
)

echo.
echo Usage example:
echo   face_enhancer.exe input_image.jpg output_image.jpg
echo.
pause