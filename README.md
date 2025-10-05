# Face Enhancer

A simple web-based image enhancement tool for improving image quality.

## Features

- 📤 **Easy Upload**: Drag & drop or click to upload images
- 🎛️ **Simple Controls**: Adjust sharpness, contrast, and brightness  
- ⚡ **Instant Processing**: Real-time enhancement using browser technology
- 👀 **Before/After**: Side-by-side comparison view
- 💾 **Download**: Save enhanced images to your computer

## Quick Start

1. **Start the web interface**:
   ```
   .\start_web.bat
   ```

2. **Open your browser** to: `http://localhost:8000/simple.html`

3. **Upload an image** by dragging & dropping or clicking the upload area

4. **Adjust settings** using the sliders:
   - **Sharpness**: Makes edges more defined (0.5 - 3.0)
   - **Contrast**: Adjusts light/dark differences (0.8 - 2.0) 
   - **Brightness**: Controls overall lightness (0.8 - 1.5)

5. **Click "Enhance Image"** to process

6. **Download** your enhanced result

## How It Works

The web interface uses HTML5 Canvas and browser-based image processing to enhance your photos:

- **Upload**: Images are loaded into the browser
- **Processing**: Canvas filters apply sharpness, contrast, and brightness adjustments
- **Enhancement**: Real-time processing using browser APIs
- **Download**: Enhanced images saved as high-quality JPEG files

## Requirements

- **Python 3.x** (usually pre-installed on Windows 10/11)
- **Modern web browser** (Chrome, Firefox, Edge, Safari)
- **Port 8000** available on your computer

## File Structure

```
face enhancer/
├── 📁 src/                          # C++ Source Code
│   ├── main.cpp                     # Application entry point
│   ├── face_enhancer.cpp            # Core enhancement engine
│   ├── image_processor.cpp          # Image I/O and quality metrics
│   ├── enhancement_algorithms.cpp   # Image processing algorithms
│   ├── face_detector.cpp            # Face detection functionality
│   ├── utils.cpp                    # Utility functions
│   └── 📁 include/                  # Header files
│       ├── face_enhancer.h
│       ├── image_processor.h
│       ├── enhancement_algorithms.h
│       ├── face_detector.h
│       └── utils.h
├── 📁 web/                          # Web Interface
│   ├── simple.html                  # Main web interface (recommended)
│   ├── index.html                   # Advanced web interface
│   ├── styles.css                   # CSS styling
│   ├── script.js                    # JavaScript functionality
│   └── python_server.py             # Python backend server
├── 📁 input/                        # Input images directory
├── 📁 output/                       # Enhanced images output
├── 📁 build/                        # Build artifacts (created after compilation)
├── CMakeLists.txt                   # C++ build configuration
├── build.bat                        # Windows build script
├── build.sh                         # Linux/macOS build script
├── start_web.bat                    # Web interface launcher
├── start_web_python.bat             # Alternative web launcher
├── LICENSE                          # MIT license
└── README.md                        # This documentation
```

## Component Overview

### 🌐 Web Interface (Recommended)
- **simple.html**: Complete browser-based face enhancer
  - Drag & drop image upload
  - Real-time enhancement sliders
  - Before/after comparison
  - Direct download functionality
- **index.html**: Advanced interface (requires backend server)
- **styles.css**: Modern, responsive styling
- **script.js**: Interactive enhancement controls

### 💻 C++ Backend (Optional)
- **main.cpp**: Command-line interface for batch processing
- **face_enhancer.cpp**: 8-step enhancement pipeline
- **image_processor.cpp**: Image I/O and quality analysis
- **enhancement_algorithms.cpp**: Core enhancement functions
- **face_detector.cpp**: OpenCV-based face detection
- **utils.cpp**: File handling and utility functions

### 🛠️ Build & Launch Tools
- **CMakeLists.txt**: Professional C++ build configuration
- **build.bat/build.sh**: Automated compilation scripts
- **start_web.bat**: One-click web interface launcher
- **python_server.py**: Simple HTTP server for advanced features

### 📂 Data Directories
- **input/**: Source images for processing
- **output/**: Enhanced images destination
- **build/**: Compiled C++ executable and artifacts

## Troubleshooting

**Web server won't start**: 
- Make sure Python is installed: `python --version`
- Check if port 8000 is free
- Try running as administrator

**Enhancement not working**:
- Use a modern browser (Chrome/Firefox recommended)
- Make sure JavaScript is enabled
- Try smaller image files (under 10MB)

**Upload fails**:
- Check file format (JPG, PNG, BMP supported)
- Ensure file size is reasonable (under 50MB)


## Performance

- **Processing Speed**: Instant (browser-based)
- **File Size Limit**: 50MB recommended
- **Quality**: High-quality JPEG output
- **Memory Usage**: Minimal (browser handles processing)

## Tips for Best Results

1. **Use good quality source images** - the tool enhances existing details
2. **Adjust gradually** - small changes often look more natural
3. **Try different combinations** - each image responds differently to settings
4. **Save originals** - always keep a backup of your original images
