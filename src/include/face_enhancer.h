#ifndef FACE_ENHANCER_H
#define FACE_ENHANCER_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/photo.hpp>
#include <string>
#include <vector>
#include <memory>

/**
 * Face Enhancement Pipeline
 * Processes blurred face images to produce clear, sharp outputs
 */
class FaceEnhancer {
public:
    struct EnhancementParams {
        // Sharpening parameters
        double sharpenStrength = 1.5;
        double sharpenRadius = 1.0;
        
        // Noise reduction parameters
        float noiseReductionStrength = 10.0f;
        float templateWindowSize = 7.0f;
        float searchWindowSize = 21.0f;
        
        // Super resolution parameters
        int srScale = 2;
        
        // Edge enhancement
        double edgeEnhancementStrength = 0.8;
        
        // Skin smoothing
        double skinSmoothingStrength = 0.3;
        
        // Brightness and contrast
        double alpha = 1.2;  // Contrast
        int beta = 10;       // Brightness
        
        // Histogram equalization
        bool useHistogramEqualization = true;
        bool useCLAHE = true;
        double claheClipLimit = 2.0;
    };

    FaceEnhancer();
    ~FaceEnhancer();

    // Main processing function
    bool enhanceImage(const std::string& inputPath, const std::string& outputPath);
    bool enhanceImage(const cv::Mat& inputImage, cv::Mat& outputImage);
    
    // Batch processing
    bool enhanceBatch(const std::string& inputDir, const std::string& outputDir);
    
    // Parameter configuration
    void setEnhancementParams(const EnhancementParams& params);
    EnhancementParams getEnhancementParams() const;
    
    // Utility functions
    bool isValidImageFormat(const std::string& filename) const;
    std::vector<std::string> getSupportedFormats() const;

private:
    EnhancementParams params_;
    cv::CascadeClassifier faceCascade_;
    std::unique_ptr<cv::dnn::Net> srNet_;
    
    // Core enhancement algorithms
    cv::Mat sharpenImage(const cv::Mat& image);
    cv::Mat reduceNoise(const cv::Mat& image);
    cv::Mat enhanceEdges(const cv::Mat& image);
    cv::Mat adjustBrightnessContrast(const cv::Mat& image);
    cv::Mat enhanceHistogram(const cv::Mat& image);
    cv::Mat smoothSkin(const cv::Mat& image, const std::vector<cv::Rect>& faces);
    cv::Mat superResolution(const cv::Mat& image);
    
    // Face detection
    std::vector<cv::Rect> detectFaces(const cv::Mat& image);
    
    // Helper functions
    bool initializeFaceDetector();
    bool initializeSuperResolution();
    cv::Mat preprocessImage(const cv::Mat& image);
    cv::Mat postprocessImage(const cv::Mat& image);
    void logProcessingStep(const std::string& step, double processingTime);
};

#endif // FACE_ENHANCER_H