#include "face_enhancer.h"
#include "image_processor.h"
#include "enhancement_algorithms.h"
#include "face_detector.h"
#include "utils.h"
#include <iostream>
#include <chrono>

FaceEnhancer::FaceEnhancer() {
    // Initialize default parameters
    params_ = EnhancementParams();
    
    // Initialize face detector and super resolution
    if (!initializeFaceDetector()) {
        Utils::logWarning("Face detector initialization failed. Face-specific enhancements will be disabled.");
    }
    
    if (!initializeSuperResolution()) {
        Utils::logWarning("Super resolution initialization failed. Using traditional upscaling methods.");
    }
}

FaceEnhancer::~FaceEnhancer() {
    // Cleanup resources
}

bool FaceEnhancer::enhanceImage(const std::string& inputPath, const std::string& outputPath) {
    try {
        Utils::logInfo("Loading image: " + inputPath);
        cv::Mat inputImage = ImageProcessor::loadImage(inputPath);
        
        if (inputImage.empty()) {
            Utils::logError("Failed to load image: " + inputPath);
            return false;
        }

        cv::Mat outputImage;
        if (!enhanceImage(inputImage, outputImage)) {
            Utils::logError("Failed to enhance image: " + inputPath);
            return false;
        }

        Utils::logInfo("Saving enhanced image: " + outputPath);
        if (!ImageProcessor::saveImage(outputImage, outputPath)) {
            Utils::logError("Failed to save image: " + outputPath);
            return false;
        }

        Utils::logInfo("Successfully enhanced image: " + inputPath + " -> " + outputPath);
        return true;

    } catch (const std::exception& e) {
        Utils::logError("Exception in enhanceImage: " + std::string(e.what()));
        return false;
    }
}

bool FaceEnhancer::enhanceImage(const cv::Mat& inputImage, cv::Mat& outputImage) {
    if (inputImage.empty()) {
        Utils::logError("Input image is empty");
        return false;
    }

    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        Utils::logInfo("Starting image enhancement pipeline");
        Utils::logInfo("Input image info: " + Utils::getImageInfo(inputImage));

        // Step 1: Preprocess image
        cv::Mat processedImage = preprocessImage(inputImage);
        logProcessingStep("Preprocessing", Utils::getElapsedTime(startTime));

        // Step 2: Detect faces for face-specific enhancements
        auto faceStartTime = std::chrono::high_resolution_clock::now();
        std::vector<cv::Rect> faces = detectFaces(processedImage);
        logProcessingStep("Face Detection", Utils::getElapsedTime(faceStartTime));
        
        Utils::logInfo("Detected " + std::to_string(faces.size()) + " face(s)");

        // Step 3: Noise reduction
        auto noiseStartTime = std::chrono::high_resolution_clock::now();
        processedImage = reduceNoise(processedImage);
        logProcessingStep("Noise Reduction", Utils::getElapsedTime(noiseStartTime));

        // Step 4: Sharpening
        auto sharpenStartTime = std::chrono::high_resolution_clock::now();
        processedImage = sharpenImage(processedImage);
        logProcessingStep("Sharpening", Utils::getElapsedTime(sharpenStartTime));

        // Step 5: Edge enhancement
        auto edgeStartTime = std::chrono::high_resolution_clock::now();
        processedImage = enhanceEdges(processedImage);
        logProcessingStep("Edge Enhancement", Utils::getElapsedTime(edgeStartTime));

        // Step 6: Brightness and contrast adjustment
        auto contrastStartTime = std::chrono::high_resolution_clock::now();
        processedImage = adjustBrightnessContrast(processedImage);
        logProcessingStep("Brightness/Contrast", Utils::getElapsedTime(contrastStartTime));

        // Step 7: Histogram enhancement
        auto histStartTime = std::chrono::high_resolution_clock::now();
        processedImage = enhanceHistogram(processedImage);
        logProcessingStep("Histogram Enhancement", Utils::getElapsedTime(histStartTime));

        // Step 8: Skin smoothing (if faces detected)
        if (!faces.empty()) {
            auto skinStartTime = std::chrono::high_resolution_clock::now();
            processedImage = smoothSkin(processedImage, faces);
            logProcessingStep("Skin Smoothing", Utils::getElapsedTime(skinStartTime));
        }

        // Step 9: Super resolution (optional)
        if (params_.srScale > 1) {
            auto srStartTime = std::chrono::high_resolution_clock::now();
            processedImage = superResolution(processedImage);
            logProcessingStep("Super Resolution", Utils::getElapsedTime(srStartTime));
        }

        // Step 10: Post-processing
        auto postStartTime = std::chrono::high_resolution_clock::now();
        outputImage = postprocessImage(processedImage);
        logProcessingStep("Post-processing", Utils::getElapsedTime(postStartTime));

        double totalTime = Utils::getElapsedTime(startTime);
        Utils::logInfo("Total enhancement time: " + std::to_string(totalTime) + " ms");
        Utils::logInfo("Output image info: " + Utils::getImageInfo(outputImage));

        return true;

    } catch (const std::exception& e) {
        Utils::logError("Exception in image enhancement pipeline: " + std::string(e.what()));
        return false;
    }
}

bool FaceEnhancer::enhanceBatch(const std::string& inputDir, const std::string& outputDir) {
    try {
        // Create output directory if it doesn't exist
        if (!Utils::createDirectory(outputDir)) {
            Utils::logError("Failed to create output directory: " + outputDir);
            return false;
        }

        // Get list of image files
        std::vector<std::string> imageFiles = Utils::listFiles(inputDir);
        std::vector<std::string> validImages;
        
        for (const auto& file : imageFiles) {
            if (isValidImageFormat(file)) {
                validImages.push_back(file);
            }
        }

        if (validImages.empty()) {
            Utils::logWarning("No valid image files found in: " + inputDir);
            return true;
        }

        Utils::logInfo("Processing " + std::to_string(validImages.size()) + " images");
        Utils::ProgressBar progress(validImages.size(), "Enhancing images");

        int successCount = 0;
        for (size_t i = 0; i < validImages.size(); ++i) {
            std::string inputPath = Utils::joinPath(inputDir, validImages[i]);
            std::string outputPath = Utils::joinPath(outputDir, "enhanced_" + validImages[i]);
            
            if (enhanceImage(inputPath, outputPath)) {
                successCount++;
            } else {
                Utils::logWarning("Failed to enhance: " + validImages[i]);
            }
            
            progress.update(i + 1);
        }

        progress.finish();
        Utils::logInfo("Batch processing completed. Successfully enhanced " + 
                      std::to_string(successCount) + "/" + std::to_string(validImages.size()) + " images");

        return successCount > 0;

    } catch (const std::exception& e) {
        Utils::logError("Exception in batch enhancement: " + std::string(e.what()));
        return false;
    }
}

void FaceEnhancer::setEnhancementParams(const EnhancementParams& params) {
    params_ = params;
    Utils::logInfo("Enhancement parameters updated");
}

FaceEnhancer::EnhancementParams FaceEnhancer::getEnhancementParams() const {
    return params_;
}

bool FaceEnhancer::isValidImageFormat(const std::string& filename) const {
    std::vector<std::string> supportedFormats = getSupportedFormats();
    std::string ext = Utils::toLowerCase(Utils::getFileExtension(filename));
    
    return std::find(supportedFormats.begin(), supportedFormats.end(), ext) != supportedFormats.end();
}

std::vector<std::string> FaceEnhancer::getSupportedFormats() const {
    return {".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".webp"};
}

// Private methods implementation

cv::Mat FaceEnhancer::sharpenImage(const cv::Mat& image) {
    return EnhancementAlgorithms::unsharpMask(image, params_.sharpenStrength, params_.sharpenRadius);
}

cv::Mat FaceEnhancer::reduceNoise(const cv::Mat& image) {
    if (image.channels() == 3) {
        return EnhancementAlgorithms::nonLocalMeansDenoising(image, 
            params_.noiseReductionStrength, 
            static_cast<int>(params_.templateWindowSize), 
            static_cast<int>(params_.searchWindowSize));
    } else {
        cv::Mat denoised;
        cv::fastNlMeansDenoising(image, denoised, params_.noiseReductionStrength);
        return denoised;
    }
}

cv::Mat FaceEnhancer::enhanceEdges(const cv::Mat& image) {
    return EnhancementAlgorithms::detailEnhance(image, 10.0, params_.edgeEnhancementStrength);
}

cv::Mat FaceEnhancer::adjustBrightnessContrast(const cv::Mat& image) {
    cv::Mat result;
    image.convertTo(result, -1, params_.alpha, params_.beta);
    return result;
}

cv::Mat FaceEnhancer::enhanceHistogram(const cv::Mat& image) {
    if (params_.useCLAHE) {
        return EnhancementAlgorithms::adaptiveHistogramEqualization(image, params_.claheClipLimit);
    } else if (params_.useHistogramEqualization) {
        cv::Mat result;
        if (image.channels() == 1) {
            cv::equalizeHist(image, result);
        } else {
            cv::cvtColor(image, result, cv::COLOR_BGR2YUV);
            std::vector<cv::Mat> channels;
            cv::split(result, channels);
            cv::equalizeHist(channels[0], channels[0]);
            cv::merge(channels, result);
            cv::cvtColor(result, result, cv::COLOR_YUV2BGR);
        }
        return result;
    }
    return image.clone();
}

cv::Mat FaceEnhancer::smoothSkin(const cv::Mat& image, const std::vector<cv::Rect>& faces) {
    if (faces.empty() || params_.skinSmoothingStrength <= 0.0) {
        return image.clone();
    }
    
    return EnhancementAlgorithms::skinSmoothing(image, faces, params_.skinSmoothingStrength);
}

cv::Mat FaceEnhancer::superResolution(const cv::Mat& image) {
    // Use traditional upscaling if DNN is not available
    return EnhancementAlgorithms::lanczosUpscale(image, params_.srScale);
}

std::vector<cv::Rect> FaceEnhancer::detectFaces(const cv::Mat& image) {
    std::vector<cv::Rect> faces;
    
    try {
        if (!faceCascade_.empty()) {
            faceCascade_.detectMultiScale(image, faces, 1.1, 3, 0, cv::Size(30, 30));
        }
    } catch (const std::exception& e) {
        Utils::logWarning("Face detection failed: " + std::string(e.what()));
    }
    
    return faces;
}

bool FaceEnhancer::initializeFaceDetector() {
    try {
        // Try to load the default Haar cascade for face detection
        std::string cascadePath = cv::samples::findFile("haarcascade_frontalface_alt.xml");
        if (cascadePath.empty()) {
            // Fallback paths
            std::vector<std::string> fallbackPaths = {
                "data/haarcascades/haarcascade_frontalface_alt.xml",
                "../data/haarcascades/haarcascade_frontalface_alt.xml",
                "../../data/haarcascades/haarcascade_frontalface_alt.xml"
            };
            
            for (const auto& path : fallbackPaths) {
                if (Utils::fileExists(path)) {
                    cascadePath = path;
                    break;
                }
            }
        }
        
        if (!cascadePath.empty() && faceCascade_.load(cascadePath)) {
            Utils::logInfo("Face detector initialized successfully");
            return true;
        } else {
            Utils::logWarning("Could not load face cascade classifier");
            return false;
        }
    } catch (const std::exception& e) {
        Utils::logError("Failed to initialize face detector: " + std::string(e.what()));
        return false;
    }
}

bool FaceEnhancer::initializeSuperResolution() {
    // For now, we'll use traditional upscaling methods
    // In a full implementation, you would load a pre-trained SR model here
    Utils::logInfo("Using traditional super resolution methods");
    return true;
}

cv::Mat FaceEnhancer::preprocessImage(const cv::Mat& image) {
    cv::Mat processed = image.clone();
    
    // Ensure image is in a standard format
    if (processed.channels() == 4) {
        cv::cvtColor(processed, processed, cv::COLOR_BGRA2BGR);
    }
    
    // Normalize to ensure consistent processing
    processed = ImageProcessor::normalizeImage(processed);
    
    return processed;
}

cv::Mat FaceEnhancer::postprocessImage(const cv::Mat& image) {
    cv::Mat processed = image.clone();
    
    // Ensure pixel values are in valid range
    cv::normalize(processed, processed, 0, 255, cv::NORM_MINMAX);
    processed.convertTo(processed, CV_8U);
    
    return processed;
}

void FaceEnhancer::logProcessingStep(const std::string& step, double processingTime) {
    Utils::logDebug(step + " completed in " + std::to_string(processingTime) + " ms");
}