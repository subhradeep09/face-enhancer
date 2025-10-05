cmake_minimum_required(VERSION 3.16)
project(FaceEnhancer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(src/include)

# Create main executable
add_executable(face_enhancer
    src/main.cpp
    src/face_enhancer.cpp
    src/image_processor.cpp
    src/enhancement_algorithms.cpp
    src/face_detector.cpp
    src/utils.cpp
)

# Link libraries
target_link_libraries(face_enhancer 
    ${OpenCV_LIBS}
    Threads::Threads
)

# Compiler-specific options
if(MSVC)
    target_compile_options(face_enhancer PRIVATE /W4)
else()
    target_compile_options(face_enhancer PRIVATE -Wall -Wextra -pedantic -O3)
endif()

# Create output directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/output)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/input)

# Copy test images if they exist
if(EXISTS ${CMAKE_SOURCE_DIR}/test_images)
    file(COPY ${CMAKE_SOURCE_DIR}/test_images DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Set runtime output directory
set_target_properties(face_enhancer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)