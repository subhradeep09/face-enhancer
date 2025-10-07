#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <string>
#include <vector>
#include <chrono>

/**
 * Enhanced Face Image Processor - C++ Implementation
 * Optimized for fast image enhancement with OpenCV
 */
class ImageEnhancer {
public:
    struct EnhancementParams {
        double scaleFactor = 2.0;
        double sharpenStrength = 1.2;
        double noiseReduction = 6.0;
        double contrast = 1.15;
        double blendWeight = 0.8;
        bool enableFaceEnhancement = true;
        std::string mode = "gfpgan";
    };

    struct EnhancementResult {
        cv::Mat enhancedImage;
        bool success = false;
        double processingTime = 0.0;
        int facesDetected = 0;
        std::string method;
        std::string error;
        
        struct Metrics {
            std::string resolution;
            std::string quality;
            std::string enhancement;
            double scaleFactor;
        } metrics;
    };

    struct ProcessingTiming {
        double upscaling = 0.0;
        double noiseReduction = 0.0;
        double sharpening = 0.0;
        double contrast = 0.0;
        double histogram = 0.0;
        double faceEnhancement = 0.0;
        double colorEnhancement = 0.0;
        double finalPolish = 0.0;
        double total = 0.0;
    };

    ImageEnhancer();
    ~ImageEnhancer();

    // Main enhancement function
    EnhancementResult enhanceImage(const cv::Mat& inputImage, const EnhancementParams& params);
    
    // Individual enhancement steps
    cv::Mat applyRealEnhancement(const cv::Mat& image, const EnhancementParams& params, ProcessingTiming& timing);
    cv::Mat enhanceFaces(const cv::Mat& image);
    
    // Utility functions
    bool isInitialized() const { return initialized_; }
    std::string getLastError() const { return lastError_; }
    
    // Image format conversion
    static cv::Mat base64ToMat(const std::string& base64Data);
    static std::string matToBase64(const cv::Mat& image, const std::string& extension = ".jpg");

private:
    bool initialized_;
    std::string lastError_;
    
    // OpenCV classifiers
    cv::CascadeClassifier faceCascade_;
    cv::CascadeClassifier eyeCascade_;
    
    // Internal enhancement functions
    cv::Mat fastUpscaling(const cv::Mat& image, double scaleFactor, const std::string& mode);
    cv::Mat optimizedNoiseReduction(const cv::Mat& image, double strength);
    cv::Mat intelligentSharpening(const cv::Mat& image, double strength);
    cv::Mat contrastEnhancement(const cv::Mat& image, double alpha);
    cv::Mat optimizedHistogramEqualization(const cv::Mat& image);
    cv::Mat colorEnhancement(const cv::Mat& image);
    cv::Mat finalPolish(const cv::Mat& image);
    
    // Face enhancement helpers
    cv::Mat enhanceSingleFace(const cv::Mat& faceRegion, const cv::Rect& faceRect);
    cv::Mat createBlendMask(const cv::Size& size, const cv::Point& center, const cv::Size& axes);
    
    // Timing utilities
    void startTimer(std::chrono::high_resolution_clock::time_point& start);
    double endTimer(const std::chrono::high_resolution_clock::time_point& start);
    
    // Initialize classifiers
    bool initializeClassifiers();
};

/**
 * Utility functions for image processing
 */
namespace ImageUtils {
    // Base64 encoding/decoding
    std::string base64_encode(const unsigned char* data, size_t len);
    std::vector<unsigned char> base64_decode(const std::string& encoded_string);
    
    // Image format detection
    std::string detectImageFormat(const std::vector<unsigned char>& data);
    
    // Performance timing
    class Timer {
    public:
        Timer() : start_(std::chrono::high_resolution_clock::now()) {}
        
        double elapsed() const {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
            return duration.count() / 1000000.0; // Convert to seconds
        }
        
        void reset() {
            start_ = std::chrono::high_resolution_clock::now();
        }
        
    private:
        std::chrono::high_resolution_clock::time_point start_;
    };
}