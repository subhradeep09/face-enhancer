#!/bin/bash

echo "========================================"
echo "    Face Enhancement C++ Server"
echo "========================================"
echo ""

# Check if already built
if [ ! -f "build/bin/FaceEnhancer" ]; then
    echo "Building the server for the first time..."
    echo ""
    chmod +x build.sh
    ./build.sh
    if [ $? -ne 0 ]; then
        echo "Build failed! Please check the errors above."
        exit 1
    fi
    echo ""
fi

# Start the server
echo "Starting Face Enhancement Server..."
echo "Server will be available at: http://localhost:8084"
echo "Press Ctrl+C to stop the server"
echo ""

cd build/bin
./FaceEnhancer

echo ""
echo "Server stopped."