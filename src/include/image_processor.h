#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

/**
 * Core image processing utilities
 */
class ImageProcessor {
public:
    // Image I/O operations
    static cv::Mat loadImage(const std::string& path);
    static bool saveImage(const cv::Mat& image, const std::string& path, int quality = 95);
    
    // Basic image operations
    static cv::Mat resizeImage(const cv::Mat& image, int width, int height, int interpolation = cv::INTER_LANCZOS4);
    static cv::Mat resizeImageProportional(const cv::Mat& image, double scaleFactor, int interpolation = cv::INTER_LANCZOS4);
    static cv::Mat cropImage(const cv::Mat& image, const cv::Rect& roi);
    
    // Image quality assessment
    static double calculateSharpness(const cv::Mat& image);
    static double calculateContrast(const cv::Mat& image);
    static double calculateBrightness(const cv::Mat& image);
    static double calculatePSNR(const cv::Mat& original, const cv::Mat& enhanced);
    static double calculateSSIM(const cv::Mat& original, const cv::Mat& enhanced);
    
    // Image format utilities
    static std::string getImageFormat(const std::string& filename);
    static bool isValidImageFile(const std::string& filename);
    static std::vector<std::string> getImagesInDirectory(const std::string& directory);
    
    // Image conversion utilities
    static cv::Mat convertToGrayscale(const cv::Mat& image);
    static cv::Mat convertToRGB(const cv::Mat& image);
    static cv::Mat normalizeImage(const cv::Mat& image);
    
    // Debug utilities
    static void displayImage(const cv::Mat& image, const std::string& windowName = "Image", int waitKey = 0);
    static void showImageComparison(const cv::Mat& original, const cv::Mat& enhanced, const std::string& title = "Comparison");
    
private:
    static std::vector<std::string> supportedFormats_;
    static bool isFormatSupported(const std::string& extension);
};

#endif // IMAGE_PROCESSOR_H