#!/bin/bash

echo "Building Face Enhancer for Linux/macOS..."

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Please install CMake."
    exit 1
fi

# Check if OpenCV is available
if ! pkg-config --exists opencv4; then
    echo "WARNING: OpenCV4 not found via pkg-config. Trying OpenCV3..."
    if ! pkg-config --exists opencv; then
        echo "ERROR: OpenCV not found. Please install OpenCV development packages."
        exit 1
    fi
fi

# Create build directory
mkdir -p build
cd build

# Configure the project
echo "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed. Please check OpenCV installation."
    exit 1
fi

# Build the project
echo "Building project..."
make -j$(nproc 2>/dev/null || echo 4)
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed."
    exit 1
fi

echo ""
echo "Build completed successfully!"
echo "Executable location: build/face_enhancer"
echo ""
echo "To run the application:"
echo "  cd build"
echo "  ./face_enhancer --help"
echo ""