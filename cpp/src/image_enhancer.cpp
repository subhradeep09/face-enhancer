#include "../include/image_enhancer.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

// Base64 encoding table
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

ImageEnhancer::ImageEnhancer() : initialized_(false) {
    if (initializeClassifiers()) {
        initialized_ = true;
        std::cout << "✅ C++ ImageEnhancer initialized successfully" << std::endl;
    } else {
        std::cout << "⚠️ Warning: Some classifiers not loaded, face enhancement may be limited" << std::endl;
        initialized_ = true; // Still functional without face detection
    }
}

ImageEnhancer::~ImageEnhancer() {
    // Cleanup is automatic with OpenCV
}

bool ImageEnhancer::initializeClassifiers() {
    try {
        // Try to load face cascade
        std::string faceCascadePath = cv::data::haarcascades + "haarcascade_frontalface_default.xml";
        if (!faceCascade_.load(faceCascadePath)) {
            std::cout << "⚠️ Could not load face cascade classifier" << std::endl;
            return false;
        }
        
        // Try to load eye cascade
        std::string eyeCascadePath = cv::data::haarcascades + "haarcascade_eye.xml";
        if (!eyeCascade_.load(eyeCascadePath)) {
            std::cout << "⚠️ Could not load eye cascade classifier" << std::endl;
            // Eye cascade is optional, don't fail
        }
        
        return true;
    } catch (const cv::Exception& e) {
        lastError_ = "Failed to initialize classifiers: " + std::string(e.what());
        return false;
    }
}

ImageEnhancer::EnhancementResult ImageEnhancer::enhanceImage(const cv::Mat& inputImage, const EnhancementParams& params) {
    EnhancementResult result;
    ProcessingTiming timing;
    
    if (inputImage.empty()) {
        result.error = "Input image is empty";
        return result;
    }
    
    try {
        std::cout << "🔧 Applying C++ OpenCV-based real enhancement..." << std::endl;
        std::cout << "📏 Original image size: " << inputImage.cols << "×" << inputImage.rows << std::endl;
        
        auto totalStart = std::chrono::high_resolution_clock::now();
        
        result.enhancedImage = applyRealEnhancement(inputImage, params, timing);
        
        auto totalEnd = std::chrono::high_resolution_clock::now();
        timing.total = std::chrono::duration_cast<std::chrono::microseconds>(totalEnd - totalStart).count() / 1000000.0;
        
        result.processingTime = timing.total;
        result.success = true;
        result.method = "C++ OpenCV Enhanced Processing";
        
        // Calculate metrics
        double actualScale = static_cast<double>(result.enhancedImage.cols) / inputImage.cols;
        result.metrics.resolution = std::to_string(inputImage.cols) + "×" + std::to_string(inputImage.rows) + 
                                   " → " + std::to_string(result.enhancedImage.cols) + "×" + std::to_string(result.enhancedImage.rows);
        result.metrics.quality = "C++ Enhanced";
        result.metrics.enhancement = params.mode + " + Face Detection";
        result.metrics.scaleFactor = actualScale;
        
        std::cout << "✅ Enhanced image size: " << result.enhancedImage.cols << "×" << result.enhancedImage.rows 
                  << " (scale: " << std::fixed << std::setprecision(1) << actualScale << "x)" << std::endl;
        std::cout << "🚀 Total C++ enhancement completed in " << std::fixed << std::setprecision(2) 
                  << timing.total << "s" << std::endl;
        
    } catch (const cv::Exception& e) {
        result.error = "OpenCV error: " + std::string(e.what());
        std::cout << "❌ C++ enhancement error: " << result.error << std::endl;
    } catch (const std::exception& e) {
        result.error = "C++ error: " + std::string(e.what());
        std::cout << "❌ C++ enhancement error: " << result.error << std::endl;
    }
    
    return result;
}

cv::Mat ImageEnhancer::applyRealEnhancement(const cv::Mat& image, const EnhancementParams& params, ProcessingTiming& timing) {
    std::cout << "🎯 Applying " << params.mode << " enhancement algorithm..." << std::endl;
    std::cout << "📊 Parameters: scale=" << params.scaleFactor << ", sharpen=" << params.sharpenStrength 
              << ", denoise=" << params.noiseReduction << std::endl;
    
    cv::Mat enhanced = image.clone();
    auto stepStart = std::chrono::high_resolution_clock::now();
    
    // 1. SUPER RESOLUTION UPSCALING
    stepStart = std::chrono::high_resolution_clock::now();
    if (params.scaleFactor > 1.0) {
        enhanced = fastUpscaling(enhanced, params.scaleFactor, params.mode);
        std::cout << "⬆️ Upscaling from " << image.cols << "×" << image.rows 
                  << " to " << enhanced.cols << "×" << enhanced.rows << std::endl;
    }
    timing.upscaling = endTimer(stepStart);
    std::cout << "⏱️ Upscaling completed in " << std::fixed << std::setprecision(2) << timing.upscaling << "s" << std::endl;
    
    // 2. OPTIMIZED NOISE REDUCTION
    stepStart = std::chrono::high_resolution_clock::now();
    if (params.noiseReduction > 0) {
        std::cout << "🔇 Applying noise reduction..." << std::endl;
        enhanced = optimizedNoiseReduction(enhanced, params.noiseReduction);
    }
    timing.noiseReduction = endTimer(stepStart);
    std::cout << "⏱️ Noise reduction completed in " << std::fixed << std::setprecision(2) << timing.noiseReduction << "s" << std::endl;
    
    // 3. INTELLIGENT SHARPENING
    stepStart = std::chrono::high_resolution_clock::now();
    if (params.sharpenStrength > 0) {
        std::cout << "🔍 Applying intelligent sharpening..." << std::endl;
        enhanced = intelligentSharpening(enhanced, params.sharpenStrength);
    }
    timing.sharpening = endTimer(stepStart);
    std::cout << "⏱️ Sharpening completed in " << std::fixed << std::setprecision(2) << timing.sharpening << "s" << std::endl;
    
    // 4. CONTRAST AND BRIGHTNESS ENHANCEMENT
    stepStart = std::chrono::high_resolution_clock::now();
    std::cout << "💡 Enhancing contrast and brightness..." << std::endl;
    enhanced = contrastEnhancement(enhanced, params.contrast);
    timing.contrast = endTimer(stepStart);
    std::cout << "⏱️ Contrast enhancement completed in " << std::fixed << std::setprecision(2) << timing.contrast << "s" << std::endl;
    
    // 5. OPTIMIZED HISTOGRAM EQUALIZATION
    stepStart = std::chrono::high_resolution_clock::now();
    std::cout << "📈 Applying optimized histogram equalization..." << std::endl;
    enhanced = optimizedHistogramEqualization(enhanced);
    timing.histogram = endTimer(stepStart);
    std::cout << "⏱️ Histogram equalization completed in " << std::fixed << std::setprecision(2) << timing.histogram << "s" << std::endl;
    
    // 6. FACE-SPECIFIC ENHANCEMENT (Optional for speed)
    stepStart = std::chrono::high_resolution_clock::now();
    if ((params.mode == "gfpgan" || params.mode == "hybrid") && params.enableFaceEnhancement) {
        std::cout << "👤 Applying face-specific enhancement..." << std::endl;
        enhanced = enhanceFaces(enhanced);
    } else {
        std::cout << "⚡ Skipping face enhancement for faster processing..." << std::endl;
    }
    timing.faceEnhancement = endTimer(stepStart);
    std::cout << "⏱️ Face enhancement completed in " << std::fixed << std::setprecision(2) << timing.faceEnhancement << "s" << std::endl;
    
    // 7. COLOR ENHANCEMENT
    stepStart = std::chrono::high_resolution_clock::now();
    std::cout << "🌈 Enhancing colors..." << std::endl;
    enhanced = colorEnhancement(enhanced);
    timing.colorEnhancement = endTimer(stepStart);
    std::cout << "⏱️ Color enhancement completed in " << std::fixed << std::setprecision(2) << timing.colorEnhancement << "s" << std::endl;
    
    // 8. OPTIMIZED FINAL POLISH
    stepStart = std::chrono::high_resolution_clock::now();
    std::cout << "✨ Applying final polish..." << std::endl;
    enhanced = finalPolish(enhanced);
    timing.finalPolish = endTimer(stepStart);
    std::cout << "⏱️ Final polish completed in " << std::fixed << std::setprecision(2) << timing.finalPolish << "s" << std::endl;
    
    return enhanced;
}

cv::Mat ImageEnhancer::fastUpscaling(const cv::Mat& image, double scaleFactor, const std::string& mode) {
    cv::Mat result;
    int newWidth = static_cast<int>(image.cols * scaleFactor);
    int newHeight = static_cast<int>(image.rows * scaleFactor);
    
    if (mode == "gfpgan") {
        // Best quality upscaling for AI mode
        cv::resize(image, result, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LANCZOS4);
    } else {
        // Good quality for traditional mode
        cv::resize(image, result, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_CUBIC);
    }
    
    return result;
}

cv::Mat ImageEnhancer::optimizedNoiseReduction(const cv::Mat& image, double strength) {
    cv::Mat result;
    // Use faster bilateral filter only for better performance
    cv::bilateralFilter(image, result, 7, strength * 6, strength * 6);
    return result;
}

cv::Mat ImageEnhancer::intelligentSharpening(const cv::Mat& image, double strength) {
    cv::Mat result;
    cv::Mat gaussian;
    
    // Unsharp mask technique
    cv::GaussianBlur(image, gaussian, cv::Size(0, 0), 2.0);
    cv::addWeighted(image, 1.0 + strength, gaussian, -strength, 0, result);
    
    // Additional edge sharpening
    cv::Mat kernel = (cv::Mat_<float>(3, 3) << -0.1, -0.1, -0.1, -0.1, 0.9, -0.1, -0.1, -0.1, -0.1);
    cv::Mat sharpened;
    cv::filter2D(result, sharpened, -1, kernel);
    cv::addWeighted(result, 0.8, sharpened, 0.2, 0, result);
    
    return result;
}

cv::Mat ImageEnhancer::contrastEnhancement(const cv::Mat& image, double alpha) {
    cv::Mat result;
    image.convertTo(result, -1, alpha, 15);
    return result;
}

cv::Mat ImageEnhancer::optimizedHistogramEqualization(const cv::Mat& image) {
    cv::Mat result;
    cv::Mat lab;
    
    cv::cvtColor(image, lab, cv::COLOR_BGR2LAB);
    
    std::vector<cv::Mat> labChannels;
    cv::split(lab, labChannels);
    
    // Apply CLAHE to L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(4, 4));
    clahe->apply(labChannels[0], labChannels[0]);
    
    cv::merge(labChannels, lab);
    cv::cvtColor(lab, result, cv::COLOR_LAB2BGR);
    
    return result;
}

cv::Mat ImageEnhancer::colorEnhancement(const cv::Mat& image) {
    cv::Mat result;
    cv::Mat hsv;
    
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    
    std::vector<cv::Mat> hsvChannels;
    cv::split(hsv, hsvChannels);
    
    // Increase saturation
    hsvChannels[1] = hsvChannels[1] * 1.15;
    // Slight brightness boost
    hsvChannels[2] = hsvChannels[2] * 1.05;
    
    cv::merge(hsvChannels, hsv);
    cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);
    
    return result;
}

cv::Mat ImageEnhancer::finalPolish(const cv::Mat& image) {
    cv::Mat result;
    cv::Mat polished;
    
    // Lighter gaussian blur to smooth artifacts (faster)
    cv::GaussianBlur(image, polished, cv::Size(3, 3), 0.3);
    cv::addWeighted(image, 0.95, polished, 0.05, 0, result);
    
    return result;
}

double ImageEnhancer::endTimer(const std::chrono::high_resolution_clock::time_point& start) {
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
}

cv::Mat ImageEnhancer::base64ToMat(const std::string& base64Data) {
    // Remove data URL prefix if present
    std::string cleanData = base64Data;
    size_t commaPos = cleanData.find(',');
    if (commaPos != std::string::npos) {
        cleanData = cleanData.substr(commaPos + 1);
    }
    
    // Decode base64
    std::vector<unsigned char> decodedData = ImageUtils::base64_decode(cleanData);
    
    // Convert to cv::Mat
    cv::Mat result = cv::imdecode(decodedData, cv::IMREAD_COLOR);
    
    return result;
}

std::string ImageEnhancer::matToBase64(const cv::Mat& image, const std::string& extension) {
    std::vector<unsigned char> buffer;
    std::vector<int> params;
    
    if (extension == ".jpg" || extension == ".jpeg") {
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(90);
    }
    
    cv::imencode(extension, image, buffer, params);
    
    std::string encoded = ImageUtils::base64_encode(buffer.data(), buffer.size());
    
    // Add data URL prefix
    std::string mimeType = (extension == ".jpg" || extension == ".jpeg") ? "image/jpeg" : "image/png";
    return "data:" + mimeType + ";base64," + encoded;
}