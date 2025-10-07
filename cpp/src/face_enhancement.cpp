#include "../include/image_enhancer.h"
#include <iostream>

cv::Mat ImageEnhancer::enhanceFaces(const cv::Mat& image) {
    try {
        std::cout << "🔍 Detecting and enhancing faces..." << std::endl;
        
        if (faceCascade_.empty()) {
            std::cout << "⚠️ Face cascade not loaded, skipping face enhancement" << std::endl;
            return image;
        }
        
        cv::Mat enhanced = image.clone();
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        
        // Detect faces with optimized parameters for speed
        std::vector<cv::Rect> faces;
        faceCascade_.detectMultiScale(gray, faces, 1.2, 3, 0, cv::Size(50, 50));
        
        int facesCount = static_cast<int>(faces.size());
        std::cout << "👥 Found " << facesCount << " face(s)" << std::endl;
        
        for (int i = 0; i < facesCount; ++i) {
            const cv::Rect& face = faces[i];
            std::cout << "🎭 Enhancing face " << (i + 1) << " at (" << face.x << ", " << face.y 
                      << ") size " << face.width << "×" << face.height << std::endl;
            
            // Extract face region with padding
            int padding = 10;
            int faceX1 = std::max(0, face.x - padding);
            int faceY1 = std::max(0, face.y - padding);
            int faceX2 = std::min(image.cols, face.x + face.width + padding);
            int faceY2 = std::min(image.rows, face.y + face.height + padding);
            
            cv::Rect expandedFace(faceX1, faceY1, faceX2 - faceX1, faceY2 - faceY1);
            cv::Mat faceRegion = enhanced(expandedFace);
            cv::Mat originalFace = faceRegion.clone();
            
            // OPTIMIZED SKIN SMOOTHING
            cv::Mat smoothed;
            cv::bilateralFilter(faceRegion, smoothed, 9, 40, 40);
            
            // EYE ENHANCEMENT (if eye cascade is available)
            if (!eyeCascade_.empty()) {
                cv::Mat faceGray;
                cv::cvtColor(faceRegion, faceGray, cv::COLOR_BGR2GRAY);
                
                std::vector<cv::Rect> eyes;
                eyeCascade_.detectMultiScale(faceGray, eyes, 1.3, 2, 0, cv::Size(20, 20));
                
                for (const cv::Rect& eye : eyes) {
                    // Sharpen eye region
                    cv::Mat eyeRegion = smoothed(eye);
                    cv::Mat sharpenedEye;
                    cv::Mat sharpeningKernel = (cv::Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
                    cv::filter2D(eyeRegion, sharpenedEye, -1, sharpeningKernel);
                    cv::addWeighted(eyeRegion, 0.6, sharpenedEye, 0.4, 0, eyeRegion);
                }
            }
            
            // MOUTH ENHANCEMENT (lower third of face)
            int mouthYStart = static_cast<int>(faceRegion.rows * 0.6);
            if (mouthYStart < faceRegion.rows) {
                cv::Mat mouthRegion = smoothed(cv::Rect(0, mouthYStart, faceRegion.cols, faceRegion.rows - mouthYStart));
                cv::Mat mouthEnhanced;
                cv::detailEnhance(mouthRegion, mouthEnhanced, 10, 0.15f);
                mouthEnhanced.copyTo(smoothed(cv::Rect(0, mouthYStart, faceRegion.cols, faceRegion.rows - mouthYStart)));
            }
            
            // BLEND ENHANCED FACE BACK
            // Create a mask for smooth blending
            cv::Mat mask = cv::Mat::zeros(faceRegion.size(), CV_8UC1);
            cv::Point center(faceRegion.cols / 2, faceRegion.rows / 2);
            cv::Size axes(faceRegion.cols / 3, static_cast<int>(faceRegion.rows / 2.5));
            cv::ellipse(mask, center, axes, 0, 0, 360, cv::Scalar(255), -1);
            
            // Smaller gaussian blur for speed
            cv::GaussianBlur(mask, mask, cv::Size(11, 11), 0);
            mask.convertTo(mask, CV_32F, 1.0 / 255.0);
            
            // Apply mask
            for (int c = 0; c < 3; ++c) {
                cv::Mat originalChannel, smoothedChannel, resultChannel;
                cv::extractChannel(originalFace, originalChannel, c);
                cv::extractChannel(smoothed, smoothedChannel, c);
                
                originalChannel.convertTo(originalChannel, CV_32F);
                smoothedChannel.convertTo(smoothedChannel, CV_32F);
                
                cv::multiply(mask, smoothedChannel, smoothedChannel);
                cv::multiply(cv::Scalar::all(1.0) - mask, originalChannel, originalChannel);
                cv::add(smoothedChannel, originalChannel, resultChannel);
                
                resultChannel.convertTo(resultChannel, CV_8U);
                cv::insertChannel(resultChannel, faceRegion, c);
            }
        }
        
        return enhanced;
        
    } catch (const cv::Exception& e) {
        std::cout << "⚠️ Face enhancement error: " << e.what() << std::endl;
        return image;
    }
}

cv::Mat ImageEnhancer::enhanceSingleFace(const cv::Mat& faceRegion, const cv::Rect& faceRect) {
    // This is a helper function that could be expanded for more sophisticated face enhancement
    cv::Mat enhanced;
    cv::bilateralFilter(faceRegion, enhanced, 15, 80, 80);
    return enhanced;
}

cv::Mat ImageEnhancer::createBlendMask(const cv::Size& size, const cv::Point& center, const cv::Size& axes) {
    cv::Mat mask = cv::Mat::zeros(size, CV_8UC1);
    cv::ellipse(mask, center, axes, 0, 0, 360, cv::Scalar(255), -1);
    cv::GaussianBlur(mask, mask, cv::Size(21, 21), 0);
    return mask;
}