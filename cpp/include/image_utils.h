#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

/**
 * Image Processing Utilities
 * Provides Base64 encoding/decoding and image format conversion utilities
 */
namespace ImageUtils {
    
    // Base64 encoding/decoding
    std::string base64Encode(const std::vector<unsigned char>& data);
    std::vector<unsigned char> base64Decode(const std::string& base64Data);
    
    // Image format conversions
    std::string imageToBase64(const cv::Mat& image, const std::string& format = ".jpg");
    cv::Mat base64ToImage(const std::string& base64Data);
    
    // Data URL handling
    std::string createDataURL(const cv::Mat& image, const std::string& mimeType = "image/jpeg");
    cv::Mat parseDataURL(const std::string& dataURL);
    std::string extractBase64FromDataURL(const std::string& dataURL);
    
    // Image validation
    bool isValidImageFormat(const std::string& filename);
    bool isValidImageData(const std::vector<unsigned char>& data);
    std::string getImageMimeType(const std::string& filename);
    
    // Image information
    struct ImageInfo {
        int width = 0;
        int height = 0;
        int channels = 0;
        std::string format;
        size_t sizeBytes = 0;
        bool isValid = false;
    };
    
    ImageInfo getImageInfo(const cv::Mat& image);
    ImageInfo getImageInfo(const std::vector<unsigned char>& data);
    
    // Utility functions
    std::vector<unsigned char> matToBytes(const cv::Mat& image, const std::string& format = ".jpg");
    cv::Mat bytesToMat(const std::vector<unsigned char>& data);
    
    // Safe image operations
    cv::Mat safeResize(const cv::Mat& image, const cv::Size& newSize, int interpolation = cv::INTER_LINEAR);
    cv::Mat safeConvert(const cv::Mat& image, int targetType);
    
    // Memory management helpers
    size_t getImageMemorySize(const cv::Mat& image);
    void optimizeImageMemory(cv::Mat& image);
}

/**
 * Base64 Implementation Details
 * Internal implementation class for Base64 operations
 */
class Base64Encoder {
public:
    static std::string encode(const unsigned char* data, size_t length);
    static std::vector<unsigned char> decode(const std::string& encoded);
    
private:
    static const std::string chars_;
    static bool isBase64(unsigned char c);
};