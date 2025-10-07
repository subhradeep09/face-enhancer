#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

/**
 * Face Enhancement Utilities
 * Provides face detection and face-specific enhancement operations
 */
class FaceEnhancer {
public:
    struct FaceDetectionResult {
        std::vector<cv::Rect> faces;
        bool success = false;
        std::string errorMessage;
        double detectionTime = 0.0;
    };

    FaceEnhancer();
    ~FaceEnhancer();

    // Face detection
    FaceDetectionResult detectFaces(const cv::Mat& image);
    bool loadCascadeClassifier(const std::string& cascadePath = "data/haarcascade_frontalface_alt.xml");
    
    // Face-specific enhancement
    cv::Mat enhanceFaceRegion(const cv::Mat& image, const cv::Rect& faceRect);
    cv::Mat enhanceMultipleFaces(const cv::Mat& image, const std::vector<cv::Rect>& faces);
    
    // Utility functions
    bool isClassifierLoaded() const { return classifierLoaded_; }
    int getMinFaceSize() const { return minFaceSize_; }
    void setMinFaceSize(int size) { minFaceSize_ = size; }
    
    double getScaleFactor() const { return scaleFactor_; }
    void setScaleFactor(double factor) { scaleFactor_ = factor; }
    
    int getMinNeighbors() const { return minNeighbors_; }
    void setMinNeighbors(int neighbors) { minNeighbors_ = neighbors; }

private:
    cv::CascadeClassifier faceClassifier_;
    bool classifierLoaded_;
    
    // Detection parameters
    int minFaceSize_;
    double scaleFactor_;
    int minNeighbors_;
    
    // Helper functions
    cv::Mat preprocessForDetection(const cv::Mat& image);
    std::vector<cv::Rect> filterDetections(const std::vector<cv::Rect>& detections, const cv::Size& imageSize);
    cv::Mat enhanceSingleFace(const cv::Mat& faceRegion);
};