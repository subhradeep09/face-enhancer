#include "face_detector.h"
#include "utils.h"
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <numeric>

FaceDetector::FaceDetector() 
    : minFaceSize_(30, 30)
    , maxFaceSize_(300, 300)
    , haarInitialized_(false)
    , lbpInitialized_(false)
    , dnnInitialized_(false) {
    
    // Try to initialize with default cascades
    initializeHaarCascade();
    initializeLBPCascade();
}

FaceDetector::~FaceDetector() {
    // Cleanup handled automatically by OpenCV
}

bool FaceDetector::initializeHaarCascade(const std::string& cascadePath) {
    try {
        std::string path = cascadePath;
        if (path.empty()) {
            path = getDefaultCascadePath("haar");
        }
        
        if (Utils::fileExists(path) && haarCascade_.load(path)) {
            haarInitialized_ = true;
            Utils::logInfo("Haar cascade loaded successfully from: " + path);
            return true;
        } else {
            Utils::logWarning("Failed to load Haar cascade from: " + path);
            return false;
        }
    } catch (const std::exception& e) {
        Utils::logError("Exception initializing Haar cascade: " + std::string(e.what()));
        return false;
    }
}

bool FaceDetector::initializeLBPCascade(const std::string& cascadePath) {
    try {
        std::string path = cascadePath;
        if (path.empty()) {
            path = getDefaultCascadePath("lbp");
        }
        
        if (Utils::fileExists(path) && lbpCascade_.load(path)) {
            lbpInitialized_ = true;
            Utils::logInfo("LBP cascade loaded successfully from: " + path);
            return true;
        } else {
            Utils::logWarning("Failed to load LBP cascade from: " + path);
            return false;
        }
    } catch (const std::exception& e) {
        Utils::logError("Exception initializing LBP cascade: " + std::string(e.what()));
        return false;
    }
}

bool FaceDetector::initializeDNNDetector(const std::string& modelPath, const std::string& configPath) {
    try {
        if (!modelPath.empty() && !configPath.empty() && 
            Utils::fileExists(modelPath) && Utils::fileExists(configPath)) {
            
            dnnNet_ = cv::dnn::readNetFromTensorflow(modelPath, configPath);
            if (!dnnNet_.empty()) {
                dnnInitialized_ = true;
                Utils::logInfo("DNN face detector loaded successfully");
                return true;
            }
        }
        
        Utils::logWarning("DNN face detector not available");
        return false;
    } catch (const std::exception& e) {
        Utils::logError("Exception initializing DNN detector: " + std::string(e.what()));
        return false;
    }
}

std::vector<cv::Rect> FaceDetector::detectFaces(const cv::Mat& image, DetectionMethod method) {
    switch (method) {
        case CASCADE_CLASSIFIER:
        case HAAR_CASCADE:
            return detectFacesHaar(image);
        case LBP_CASCADE:
            return detectFacesLBP(image);
        case DNN_FACE_DETECTION:
            return detectFacesDNN(image);
        default:
            return detectFacesHaar(image);
    }
}

std::vector<cv::Rect> FaceDetector::detectFacesHaar(const cv::Mat& image, double scaleFactor, int minNeighbors) {
    if (image.empty() || !haarInitialized_) {
        Utils::logWarning("Image empty or Haar cascade not initialized");
        return {};
    }

    try {
        std::vector<cv::Rect> faces;
        cv::Mat gray;
        
        if (image.channels() > 1) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image.clone();
        }
        
        // Enhance contrast for better detection
        cv::equalizeHist(gray, gray);
        
        haarCascade_.detectMultiScale(
            gray, 
            faces,
            scaleFactor,
            minNeighbors,
            cv::CASCADE_SCALE_IMAGE,
            minFaceSize_,
            maxFaceSize_
        );
        
        Utils::logDebug("Haar detection found " + std::to_string(faces.size()) + " faces");
        return filterOverlappingRects(faces);
        
    } catch (const std::exception& e) {
        Utils::logError("Exception in Haar face detection: " + std::string(e.what()));
        return {};
    }
}

std::vector<cv::Rect> FaceDetector::detectFacesLBP(const cv::Mat& image, double scaleFactor, int minNeighbors) {
    if (image.empty() || !lbpInitialized_) {
        Utils::logWarning("Image empty or LBP cascade not initialized");
        return {};
    }

    try {
        std::vector<cv::Rect> faces;
        cv::Mat gray;
        
        if (image.channels() > 1) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image.clone();
        }
        
        // Enhance contrast
        cv::equalizeHist(gray, gray);
        
        lbpCascade_.detectMultiScale(
            gray, 
            faces,
            scaleFactor,
            minNeighbors,
            cv::CASCADE_SCALE_IMAGE,
            minFaceSize_,
            maxFaceSize_
        );
        
        Utils::logDebug("LBP detection found " + std::to_string(faces.size()) + " faces");
        return filterOverlappingRects(faces);
        
    } catch (const std::exception& e) {
        Utils::logError("Exception in LBP face detection: " + std::string(e.what()));
        return {};
    }
}

std::vector<cv::Rect> FaceDetector::detectFacesDNN(const cv::Mat& image, float confidenceThreshold) {
    if (image.empty() || !dnnInitialized_) {
        Utils::logWarning("Image empty or DNN not initialized");
        return {};
    }

    try {
        cv::Mat blob = preprocessForDNN(image);
        dnnNet_.setInput(blob);
        
        cv::Mat detections = dnnNet_.forward();
        return postprocessDNNResults(detections, image.size(), confidenceThreshold);
        
    } catch (const std::exception& e) {
        Utils::logError("Exception in DNN face detection: " + std::string(e.what()));
        return {};
    }
}

cv::Mat FaceDetector::extractFaceRegion(const cv::Mat& image, const cv::Rect& faceRect, int padding) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Rect expandedRect = expandRect(faceRect, image.size(), padding);
        return image(expandedRect).clone();
    } catch (const std::exception& e) {
        Utils::logError("Exception extracting face region: " + std::string(e.what()));
        return cv::Mat();
    }
}

std::vector<cv::Mat> FaceDetector::extractAllFaces(const cv::Mat& image, const std::vector<cv::Rect>& faceRects) {
    std::vector<cv::Mat> faces;
    
    for (const auto& rect : faceRects) {
        cv::Mat face = extractFaceRegion(image, rect);
        if (!face.empty()) {
            faces.push_back(face);
        }
    }
    
    return faces;
}

double FaceDetector::assessFaceQuality(const cv::Mat& faceImage) {
    if (faceImage.empty()) return 0.0;

    try {
        double sharpness = 0.0;
        double brightness = 0.0;
        double contrast = 0.0;
        
        // Convert to grayscale for analysis
        cv::Mat gray;
        if (faceImage.channels() > 1) {
            cv::cvtColor(faceImage, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = faceImage.clone();
        }
        
        // Calculate sharpness using Laplacian variance
        cv::Mat laplacian;
        cv::Laplacian(gray, laplacian, CV_64F);
        cv::Scalar mean, stddev;
        cv::meanStdDev(laplacian, mean, stddev);
        sharpness = stddev.val[0] * stddev.val[0];
        
        // Calculate brightness
        cv::Scalar meanBrightness = cv::mean(gray);
        brightness = meanBrightness.val[0];
        
        // Calculate contrast
        cv::meanStdDev(gray, mean, stddev);
        contrast = stddev.val[0];
        
        // Normalize and combine metrics (simple weighted average)
        double normalizedSharpness = std::min(sharpness / 1000.0, 1.0);
        double normalizedBrightness = 1.0 - std::abs(brightness - 128.0) / 128.0;
        double normalizedContrast = std::min(contrast / 64.0, 1.0);
        
        return (normalizedSharpness * 0.5 + normalizedBrightness * 0.3 + normalizedContrast * 0.2);
        
    } catch (const std::exception& e) {
        Utils::logError("Exception assessing face quality: " + std::string(e.what()));
        return 0.0;
    }
}

bool FaceDetector::isFaceBlurred(const cv::Mat& faceImage, double threshold) {
    if (faceImage.empty()) return true;

    try {
        cv::Mat gray, laplacian;
        
        if (faceImage.channels() > 1) {
            cv::cvtColor(faceImage, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = faceImage.clone();
        }
        
        cv::Laplacian(gray, laplacian, CV_64F);
        cv::Scalar mean, stddev;
        cv::meanStdDev(laplacian, mean, stddev);
        
        double variance = stddev.val[0] * stddev.val[0];
        return variance < threshold;
        
    } catch (const std::exception& e) {
        Utils::logError("Exception checking if face is blurred: " + std::string(e.what()));
        return true;
    }
}

bool FaceDetector::isFaceWellLit(const cv::Mat& faceImage, double minBrightness, double maxBrightness) {
    if (faceImage.empty()) return false;

    try {
        cv::Mat gray;
        
        if (faceImage.channels() > 1) {
            cv::cvtColor(faceImage, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = faceImage.clone();
        }
        
        cv::Scalar meanBrightness = cv::mean(gray);
        double brightness = meanBrightness.val[0];
        
        return brightness >= minBrightness && brightness <= maxBrightness;
        
    } catch (const std::exception& e) {
        Utils::logError("Exception checking face lighting: " + std::string(e.what()));
        return false;
    }
}

cv::Mat FaceDetector::drawFaceBoxes(const cv::Mat& image, const std::vector<cv::Rect>& faces) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat result = image.clone();
        
        for (size_t i = 0; i < faces.size(); ++i) {
            cv::rectangle(result, faces[i], cv::Scalar(0, 255, 0), 2);
            
            // Add face number
            std::string label = "Face " + std::to_string(i + 1);
            cv::putText(result, label, 
                       cv::Point(faces[i].x, faces[i].y - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        }
        
        return result;
    } catch (const std::exception& e) {
        Utils::logError("Exception drawing face boxes: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat FaceDetector::drawFaceLandmarks(const cv::Mat& image, const std::vector<cv::Point2f>& landmarks) {
    if (image.empty() || landmarks.empty()) return image.clone();

    try {
        cv::Mat result = image.clone();
        
        for (const auto& point : landmarks) {
            cv::circle(result, point, 2, cv::Scalar(0, 0, 255), -1);
        }
        
        return result;
    } catch (const std::exception& e) {
        Utils::logError("Exception drawing landmarks: " + std::string(e.what()));
        return image.clone();
    }
}

// Private helper methods

std::vector<cv::Rect> FaceDetector::filterOverlappingRects(const std::vector<cv::Rect>& rects, double overlapThreshold) {
    if (rects.empty()) return {};

    try {
        std::vector<cv::Rect> filtered;
        std::vector<bool> suppressed(rects.size(), false);
        
        for (size_t i = 0; i < rects.size(); ++i) {
            if (suppressed[i]) continue;
            
            filtered.push_back(rects[i]);
            
            for (size_t j = i + 1; j < rects.size(); ++j) {
                if (suppressed[j]) continue;
                
                cv::Rect intersection = rects[i] & rects[j];
                double overlap = static_cast<double>(intersection.area()) / 
                               std::min(rects[i].area(), rects[j].area());
                
                if (overlap > overlapThreshold) {
                    suppressed[j] = true;
                }
            }
        }
        
        return filtered;
    } catch (const std::exception& e) {
        Utils::logError("Exception filtering overlapping rectangles: " + std::string(e.what()));
        return rects;
    }
}

cv::Rect FaceDetector::expandRect(const cv::Rect& rect, const cv::Size& imageSize, int padding) {
    cv::Rect expanded(
        std::max(0, rect.x - padding),
        std::max(0, rect.y - padding),
        std::min(imageSize.width - std::max(0, rect.x - padding), rect.width + 2 * padding),
        std::min(imageSize.height - std::max(0, rect.y - padding), rect.height + 2 * padding)
    );
    
    return expanded;
}

std::string FaceDetector::getDefaultCascadePath(const std::string& cascadeType) {
    std::vector<std::string> searchPaths;
    
    if (cascadeType == "haar") {
        searchPaths = {
            "data/haarcascades/haarcascade_frontalface_alt.xml",
            "../data/haarcascades/haarcascade_frontalface_alt.xml",
            "../../data/haarcascades/haarcascade_frontalface_alt.xml",
            cv::samples::findFile("haarcascade_frontalface_alt.xml")
        };
    } else if (cascadeType == "lbp") {
        searchPaths = {
            "data/lbpcascades/lbpcascade_frontalface.xml",
            "../data/lbpcascades/lbpcascade_frontalface.xml",
            "../../data/lbpcascades/lbpcascade_frontalface.xml",
            cv::samples::findFile("lbpcascade_frontalface.xml")
        };
    }
    
    for (const auto& path : searchPaths) {
        if (!path.empty() && Utils::fileExists(path)) {
            return path;
        }
    }
    
    return "";
}

cv::Mat FaceDetector::preprocessForDNN(const cv::Mat& image, const cv::Size& inputSize) {
    cv::Mat blob;
    cv::dnn::blobFromImage(image, blob, 1.0, inputSize, cv::Scalar(104, 117, 123), false, false);
    return blob;
}

std::vector<cv::Rect> FaceDetector::postprocessDNNResults(const cv::Mat& detections, const cv::Size& imageSize, float confidenceThreshold) {
    std::vector<cv::Rect> faces;
    
    try {
        cv::Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());
        
        for (int i = 0; i < detectionMat.rows; ++i) {
            float confidence = detectionMat.at<float>(i, 2);
            
            if (confidence > confidenceThreshold) {
                int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * imageSize.width);
                int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * imageSize.height);
                int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * imageSize.width);
                int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * imageSize.height);
                
                cv::Rect face(x1, y1, x2 - x1, y2 - y1);
                
                // Ensure face is within image bounds
                face = face & cv::Rect(0, 0, imageSize.width, imageSize.height);
                
                if (face.area() > 0) {
                    faces.push_back(face);
                }
            }
        }
    } catch (const std::exception& e) {
        Utils::logError("Exception in DNN postprocessing: " + std::string(e.what()));
    }
    
    return faces;
}

std::vector<cv::Point2f> FaceDetector::detectFaceLandmarks(const cv::Mat& image, const cv::Rect& faceRect) {
    // This would require a separate landmark detection model
    // For now, return empty vector
    Utils::logWarning("Face landmark detection not implemented");
    return {};
}