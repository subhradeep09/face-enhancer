#!/bin/bash

echo "=== Building Face Enhancement C++ Server ==="

# Create build directory
mkdir -p build
cd build

# Configure CMake
echo "Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed!"
    echo "Please ensure:"
    echo "1. CMake is installed"
    echo "2. OpenCV is installed and can be found by CMake"
    echo "3. GCC/G++ compiler is available"
    exit 1
fi

# Build project
echo "Building project..."
cmake --build . --config Release -- -j$(nproc)

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "=== Build Complete ==="
echo "Executable: build/bin/FaceEnhancer"
echo ""
echo "To run the server:"
echo "cd build/bin"
echo "./FaceEnhancer"
echo ""