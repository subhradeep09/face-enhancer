#include "face_enhancer.h"
#include "image_processor.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <chrono>

void printUsage(const std::string& programName) {
    std::cout << "\n=== Face Enhancer - C++ Image Enhancement Tool ===\n\n";
    std::cout << "DESCRIPTION:\n";
    std::cout << "  Enhances blurred face images to produce clear, sharp outputs using\n";
    std::cout << "  advanced computer vision algorithms including noise reduction,\n";
    std::cout << "  sharpening, edge enhancement, and super resolution.\n\n";
    
    std::cout << "USAGE:\n";
    std::cout << "  " << programName << " [OPTIONS]\n\n";
    
    std::cout << "OPTIONS:\n";
    std::cout << "  -i, --input PATH      Input image file or directory\n";
    std::cout << "  -o, --output PATH     Output file or directory\n";
    std::cout << "  -b, --batch           Process all images in input directory\n";
    std::cout << "  -c, --config FILE     Load configuration from file\n";
    std::cout << "  -v, --verbose         Enable verbose logging\n";
    std::cout << "  -p, --preview         Show before/after comparison\n";
    std::cout << "  -h, --help            Show this help message\n";
    std::cout << "      --info            Show system information\n\n";
    
    std::cout << "ENHANCEMENT PARAMETERS:\n";
    std::cout << "  --sharpen FLOAT       Sharpening strength (default: 1.5)\n";
    std::cout << "  --denoise FLOAT       Noise reduction strength (default: 10.0)\n";
    std::cout << "  --contrast FLOAT      Contrast adjustment (default: 1.2)\n";
    std::cout << "  --brightness INT      Brightness adjustment (default: 10)\n";
    std::cout << "  --scale INT           Super resolution scale (default: 1)\n\n";
    
    std::cout << "EXAMPLES:\n";
    std::cout << "  # Enhance single image\n";
    std::cout << "  " << programName << " -i blurred_face.jpg -o enhanced_face.jpg\n\n";
    std::cout << "  # Batch process directory\n";
    std::cout << "  " << programName << " -i input_dir -o output_dir --batch\n\n";
    std::cout << "  # Custom enhancement settings\n";
    std::cout << "  " << programName << " -i input.jpg -o output.jpg --sharpen 2.0 --denoise 15.0\n\n";
    
    std::cout << "SUPPORTED FORMATS:\n";
    std::cout << "  Input:  JPG, JPEG, PNG, BMP, TIFF, TIF, WEBP\n";
    std::cout << "  Output: JPG, PNG, BMP, TIFF, WEBP\n\n";
}

void printVersion() {
    std::cout << "Face Enhancer v1.0.0\n";
    std::cout << "Built with OpenCV " << CV_VERSION << "\n";
    std::cout << "Copyright (c) 2025 Face Enhancer Project\n\n";
}

bool parseArguments(int argc, char* argv[], Utils::Config& config, FaceEnhancer::EnhancementParams& params) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }
        else if (arg == "--version") {
            printVersion();
            return false;
        }
        else if (arg == "--info") {
            Utils::printSystemInfo();
            return false;
        }
        else if (arg == "-v" || arg == "--verbose") {
            config.verbose = true;
            Utils::setLogLevel(Utils::LOG_DEBUG);
        }
        else if (arg == "-b" || arg == "--batch") {
            config.batchMode = true;
        }
        else if (arg == "-p" || arg == "--preview") {
            config.showPreview = true;
        }
        else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            config.inputPath = argv[++i];
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            config.outputPath = argv[++i];
        }
        else if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
            std::string configPath = argv[++i];
            config = Utils::Config::loadFromFile(configPath);
        }
        else if (arg == "--sharpen" && i + 1 < argc) {
            params.sharpenStrength = std::stod(argv[++i]);
        }
        else if (arg == "--denoise" && i + 1 < argc) {
            params.noiseReductionStrength = std::stof(argv[++i]);
        }
        else if (arg == "--contrast" && i + 1 < argc) {
            params.alpha = std::stod(argv[++i]);
        }
        else if (arg == "--brightness" && i + 1 < argc) {
            params.beta = std::stoi(argv[++i]);
        }
        else if (arg == "--scale" && i + 1 < argc) {
            params.srScale = std::stoi(argv[++i]);
        }
        else if (arg.substr(0, 2) == "--") {
            Utils::logWarning("Unknown option: " + arg);
        }
        else {
            Utils::logWarning("Unknown argument: " + arg);
        }
    }
    
    return true;
}

bool validateInputs(const Utils::Config& config) {
    if (config.inputPath.empty()) {
        Utils::logError("Input path is required. Use -i or --input to specify.");
        return false;
    }
    
    if (config.outputPath.empty()) {
        Utils::logError("Output path is required. Use -o or --output to specify.");
        return false;
    }
    
    if (config.batchMode) {
        if (!Utils::directoryExists(config.inputPath)) {
            Utils::logError("Input directory does not exist: " + config.inputPath);
            return false;
        }
    } else {
        if (!Utils::fileExists(config.inputPath)) {
            Utils::logError("Input file does not exist: " + config.inputPath);
            return false;
        }
        
        if (!ImageProcessor::isValidImageFile(config.inputPath)) {
            Utils::logError("Input file is not a valid image format: " + config.inputPath);
            return false;
        }
    }
    
    return true;
}

void printEnhancementSummary(const Utils::Config& config, const FaceEnhancer::EnhancementParams& params) {
    Utils::logInfo("=== Enhancement Summary ===");
    Utils::logInfo("Mode: " + std::string(config.batchMode ? "Batch processing" : "Single image"));
    Utils::logInfo("Input: " + config.inputPath);
    Utils::logInfo("Output: " + config.outputPath);
    Utils::logInfo("Sharpen strength: " + std::to_string(params.sharpenStrength));
    Utils::logInfo("Noise reduction: " + std::to_string(params.noiseReductionStrength));
    Utils::logInfo("Contrast: " + std::to_string(params.alpha));
    Utils::logInfo("Brightness: " + std::to_string(params.beta));
    if (params.srScale > 1) {
        Utils::logInfo("Super resolution scale: " + std::to_string(params.srScale));
    }
    Utils::logInfo("==========================");
}

int main(int argc, char* argv[]) {
    try {
        // Print application header
        std::cout << "\n";
        Utils::logInfo("Face Enhancer - Advanced Image Enhancement Tool");
        Utils::logInfo("Initializing...");
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Initialize configuration with defaults
        Utils::Config config;
        config.inputPath = "";
        config.outputPath = "";
        config.batchMode = false;
        config.verbose = false;
        config.showPreview = false;
        config.sharpenStrength = 1.5;
        config.noiseReduction = 10.0;
        config.superResolutionScale = 1;
        
        // Initialize enhancement parameters with defaults
        FaceEnhancer::EnhancementParams params;
        
        // Parse command line arguments
        if (!parseArguments(argc, argv, config, params)) {
            return 0; // Help was shown or error occurred
        }
        
        // Set log level based on verbose flag
        if (config.verbose) {
            Utils::setLogLevel(Utils::LOG_DEBUG);
        }
        
        // Validate inputs
        if (!validateInputs(config)) {
            Utils::logError("Input validation failed. Use --help for usage information.");
            return 1;
        }
        
        // Print enhancement summary
        printEnhancementSummary(config, params);
        
        // Initialize face enhancer
        Utils::logInfo("Initializing Face Enhancer...");
        FaceEnhancer enhancer;
        enhancer.setEnhancementParams(params);
        
        bool success = false;
        
        if (config.batchMode) {
            // Batch processing
            Utils::logInfo("Starting batch processing...");
            success = enhancer.enhanceBatch(config.inputPath, config.outputPath);
        } else {
            // Single image processing
            Utils::logInfo("Processing single image...");
            
            if (config.showPreview) {
                // Load and show original image
                cv::Mat originalImage = ImageProcessor::loadImage(config.inputPath);
                if (!originalImage.empty()) {
                    cv::Mat enhancedImage;
                    if (enhancer.enhanceImage(originalImage, enhancedImage)) {
                        ImageProcessor::showImageComparison(originalImage, enhancedImage, "Face Enhancement Result");
                        success = ImageProcessor::saveImage(enhancedImage, config.outputPath);
                    }
                }
            } else {
                success = enhancer.enhanceImage(config.inputPath, config.outputPath);
            }
        }
        
        // Calculate and display total processing time
        double totalTime = Utils::getElapsedTime(startTime);
        
        if (success) {
            Utils::logInfo("Enhancement completed successfully!");
            Utils::logInfo("Total processing time: " + std::to_string(totalTime) + " ms");
            
            if (!config.batchMode) {
                // Display image quality metrics for single image
                cv::Mat original = ImageProcessor::loadImage(config.inputPath);
                cv::Mat enhanced = ImageProcessor::loadImage(config.outputPath);
                
                if (!original.empty() && !enhanced.empty()) {
                    double psnr = ImageProcessor::calculatePSNR(original, enhanced);
                    double ssim = ImageProcessor::calculateSSIM(original, enhanced);
                    double sharpnessOrig = ImageProcessor::calculateSharpness(original);
                    double sharpnessEnh = ImageProcessor::calculateSharpness(enhanced);
                    
                    Utils::logInfo("=== Quality Metrics ===");
                    Utils::logInfo("PSNR: " + std::to_string(psnr) + " dB");
                    Utils::logInfo("SSIM: " + std::to_string(ssim));
                    Utils::logInfo("Sharpness improvement: " + 
                                  std::to_string(sharpnessEnh / sharpnessOrig) + "x");
                    Utils::logInfo("=====================");
                }
            }
            
            return 0;
        } else {
            Utils::logError("Enhancement failed!");
            return 1;
        }
        
    } catch (const Utils::FaceEnhancerException& e) {
        Utils::logError("Face Enhancer Error: " + std::string(e.what()));
        return 1;
    } catch (const std::exception& e) {
        Utils::logError("Unexpected error: " + std::string(e.what()));
        return 1;
    } catch (...) {
        Utils::logError("Unknown error occurred");
        return 1;
    }
}