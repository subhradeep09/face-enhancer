#ifndef FACE_DETECTOR_H
#define FACE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

/**
 * Face detection utilities using multiple methods
 */
class FaceDetector {
public:
    enum DetectionMethod {
        CASCADE_CLASSIFIER,
        DNN_FACE_DETECTION,
        HAAR_CASCADE,
        LBP_CASCADE
    };

    FaceDetector();
    ~FaceDetector();

    // Initialization methods
    bool initializeHaarCascade(const std::string& cascadePath = "");
    bool initializeLBPCascade(const std::string& cascadePath = "");
    bool initializeDNNDetector(const std::string& modelPath = "", const std::string& configPath = "");

    // Face detection methods
    std::vector<cv::Rect> detectFaces(const cv::Mat& image, DetectionMethod method = CASCADE_CLASSIFIER);
    std::vector<cv::Rect> detectFacesHaar(const cv::Mat& image, double scaleFactor = 1.1, int minNeighbors = 3);
    std::vector<cv::Rect> detectFacesLBP(const cv::Mat& image, double scaleFactor = 1.1, int minNeighbors = 3);
    std::vector<cv::Rect> detectFacesDNN(const cv::Mat& image, float confidenceThreshold = 0.5);

    // Face landmark detection
    std::vector<cv::Point2f> detectFaceLandmarks(const cv::Mat& image, const cv::Rect& faceRect);

    // Utility functions
    cv::Mat extractFaceRegion(const cv::Mat& image, const cv::Rect& faceRect, int padding = 20);
    std::vector<cv::Mat> extractAllFaces(const cv::Mat& image, const std::vector<cv::Rect>& faceRects);
    
    // Face quality assessment
    double assessFaceQuality(const cv::Mat& faceImage);
    bool isFaceBlurred(const cv::Mat& faceImage, double threshold = 100.0);
    bool isFaceWellLit(const cv::Mat& faceImage, double minBrightness = 50.0, double maxBrightness = 200.0);

    // Visualization
    cv::Mat drawFaceBoxes(const cv::Mat& image, const std::vector<cv::Rect>& faces);
    cv::Mat drawFaceLandmarks(const cv::Mat& image, const std::vector<cv::Point2f>& landmarks);

    // Configuration
    void setMinFaceSize(const cv::Size& minSize) { minFaceSize_ = minSize; }
    void setMaxFaceSize(const cv::Size& maxSize) { maxFaceSize_ = maxSize; }
    cv::Size getMinFaceSize() const { return minFaceSize_; }
    cv::Size getMaxFaceSize() const { return maxFaceSize_; }

private:
    cv::CascadeClassifier haarCascade_;
    cv::CascadeClassifier lbpCascade_;
    cv::dnn::Net dnnNet_;
    
    cv::Size minFaceSize_;
    cv::Size maxFaceSize_;
    
    bool haarInitialized_;
    bool lbpInitialized_;
    bool dnnInitialized_;

    // Helper functions
    std::vector<cv::Rect> filterOverlappingRects(const std::vector<cv::Rect>& rects, double overlapThreshold = 0.3);
    cv::Rect expandRect(const cv::Rect& rect, const cv::Size& imageSize, int padding);
    std::string getDefaultCascadePath(const std::string& cascadeType);
    
    // DNN preprocessing
    cv::Mat preprocessForDNN(const cv::Mat& image, const cv::Size& inputSize = cv::Size(300, 300));
    std::vector<cv::Rect> postprocessDNNResults(const cv::Mat& detections, const cv::Size& imageSize, float confidenceThreshold);
};

#endif // FACE_DETECTOR_H