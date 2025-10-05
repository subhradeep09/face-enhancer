#include "enhancement_algorithms.h"
#include "utils.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/ximgproc.hpp>
#include <cmath>
#include <algorithm>

cv::Mat EnhancementAlgorithms::unsharpMask(const cv::Mat& image, double strength, double radius, double threshold) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat blurred, mask, sharpened;
        
        // Create Gaussian blur
        int kernelSize = static_cast<int>(2 * ceil(3 * radius) + 1);
        cv::GaussianBlur(image, blurred, cv::Size(kernelSize, kernelSize), radius);
        
        // Create unsharp mask
        cv::subtract(image, blurred, mask);
        
        // Apply threshold if specified
        if (threshold > 0) {
            cv::threshold(cv::abs(mask), mask, threshold, 255, cv::THRESH_TOZERO);
        }
        
        // Add weighted mask to original
        cv::addWeighted(image, 1.0, mask, strength, 0, sharpened);
        
        return sharpened;
    } catch (const std::exception& e) {
        Utils::logError("Exception in unsharp mask: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::laplacianSharpen(const cv::Mat& image, double strength) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat laplacian, sharpened;
        
        // Create Laplacian kernel
        cv::Mat kernel = (cv::Mat_<float>(3, 3) << 
            0, -1, 0,
            -1, 5, -1,
            0, -1, 0);
        
        // Apply filter
        cv::filter2D(image, sharpened, -1, kernel * strength);
        
        return sharpened;
    } catch (const std::exception& e) {
        Utils::logError("Exception in Laplacian sharpen: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::highPassSharpen(const cv::Mat& image, double strength) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat blurred, highpass, sharpened;
        
        // Create low-pass filtered version
        cv::GaussianBlur(image, blurred, cv::Size(0, 0), 2.0);
        
        // High-pass = original - low-pass
        cv::subtract(image, blurred, highpass);
        
        // Add high-pass to original
        cv::addWeighted(image, 1.0, highpass, strength, 0, sharpened);
        
        return sharpened;
    } catch (const std::exception& e) {
        Utils::logError("Exception in high-pass sharpen: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::bilateralFilter(const cv::Mat& image, int d, double sigmaColor, double sigmaSpace) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat filtered;
        cv::bilateralFilter(image, filtered, d, sigmaColor, sigmaSpace);
        return filtered;
    } catch (const std::exception& e) {
        Utils::logError("Exception in bilateral filter: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::nonLocalMeansDenoising(const cv::Mat& image, float h, int templateWindowSize, int searchWindowSize) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat denoised;
        
        if (image.channels() == 1) {
            cv::fastNlMeansDenoising(image, denoised, h, templateWindowSize, searchWindowSize);
        } else {
            cv::fastNlMeansDenoisingColored(image, denoised, h, h, templateWindowSize, searchWindowSize);
        }
        
        return denoised;
    } catch (const std::exception& e) {
        Utils::logError("Exception in non-local means denoising: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::guidedFilter(const cv::Mat& image, const cv::Mat& guide, int radius, double eps) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat filtered;
        cv::ximgproc::guidedFilter(guide.empty() ? image : guide, image, filtered, radius, eps);
        return filtered;
    } catch (const std::exception& e) {
        Utils::logError("Exception in guided filter: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::edgePreservingFilter(const cv::Mat& image, int flags, double sigmaS, double sigmaR) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat filtered;
        cv::edgePreservingFilter(image, filtered, flags, static_cast<float>(sigmaS), static_cast<float>(sigmaR));
        return filtered;
    } catch (const std::exception& e) {
        Utils::logError("Exception in edge preserving filter: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::detailEnhance(const cv::Mat& image, double sigmaS, double sigmaR) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat enhanced;
        cv::detailEnhance(image, enhanced, static_cast<float>(sigmaS), static_cast<float>(sigmaR));
        return enhanced;
    } catch (const std::exception& e) {
        Utils::logError("Exception in detail enhance: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::pencilSketch(const cv::Mat& image, double sigmaS, double sigmaR, double shadeFactor) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat gray, colorized;
        cv::pencilSketch(image, gray, colorized, static_cast<float>(sigmaS), static_cast<float>(sigmaR), static_cast<float>(shadeFactor));
        return colorized;
    } catch (const std::exception& e) {
        Utils::logError("Exception in pencil sketch: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::adaptiveHistogramEqualization(const cv::Mat& image, double clipLimit, const cv::Size& tileGridSize) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat result;
        
        if (image.channels() == 1) {
            auto clahe = cv::createCLAHE(clipLimit, tileGridSize);
            clahe->apply(image, result);
        } else {
            cv::cvtColor(image, result, cv::COLOR_BGR2LAB);
            std::vector<cv::Mat> channels;
            cv::split(result, channels);
            
            auto clahe = cv::createCLAHE(clipLimit, tileGridSize);
            clahe->apply(channels[0], channels[0]);
            
            cv::merge(channels, result);
            cv::cvtColor(result, result, cv::COLOR_LAB2BGR);
        }
        
        return result;
    } catch (const std::exception& e) {
        Utils::logError("Exception in adaptive histogram equalization: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::gammaCorrection(const cv::Mat& image, double gamma) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat lookupTable(1, 256, CV_8U);
        uchar* p = lookupTable.ptr();
        for (int i = 0; i < 256; ++i) {
            p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
        }
        
        cv::Mat result;
        cv::LUT(image, lookupTable, result);
        return result;
    } catch (const std::exception& e) {
        Utils::logError("Exception in gamma correction: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::retinexSSR(const cv::Mat& image, double sigma) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat imageFloat, blurred, retinex;
        image.convertTo(imageFloat, CV_32F, 1.0/255.0);
        
        // Add small constant to avoid log(0)
        imageFloat += 0.001;
        
        // Gaussian blur for surround function
        cv::GaussianBlur(imageFloat, blurred, cv::Size(0, 0), sigma);
        blurred += 0.001;
        
        // Retinex = log(image) - log(blurred)
        cv::log(imageFloat, imageFloat);
        cv::log(blurred, blurred);
        cv::subtract(imageFloat, blurred, retinex);
        
        // Normalize and convert back
        cv::normalize(retinex, retinex, 0, 255, cv::NORM_MINMAX);
        retinex.convertTo(retinex, CV_8U);
        
        return retinex;
    } catch (const std::exception& e) {
        Utils::logError("Exception in Retinex SSR: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::retinexMSR(const cv::Mat& image, const std::vector<double>& sigmas) {
    if (image.empty() || sigmas.empty()) return cv::Mat();

    try {
        cv::Mat result = cv::Mat::zeros(image.size(), CV_32F);
        
        for (double sigma : sigmas) {
            cv::Mat ssr = retinexSSR(image, sigma);
            ssr.convertTo(ssr, CV_32F);
            result += ssr;
        }
        
        result /= static_cast<float>(sigmas.size());
        result.convertTo(result, CV_8U);
        
        return result;
    } catch (const std::exception& e) {
        Utils::logError("Exception in Retinex MSR: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::bicubicUpscale(const cv::Mat& image, int scale) {
    if (image.empty() || scale <= 1) return image.clone();

    try {
        cv::Mat upscaled;
        cv::resize(image, upscaled, cv::Size(), scale, scale, cv::INTER_CUBIC);
        return upscaled;
    } catch (const std::exception& e) {
        Utils::logError("Exception in bicubic upscale: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::lanczosUpscale(const cv::Mat& image, int scale) {
    if (image.empty() || scale <= 1) return image.clone();

    try {
        cv::Mat upscaled;
        cv::resize(image, upscaled, cv::Size(), scale, scale, cv::INTER_LANCZOS4);
        return upscaled;
    } catch (const std::exception& e) {
        Utils::logError("Exception in Lanczos upscale: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::edgeDirectedInterpolation(const cv::Mat& image, int scale) {
    // Simplified edge-directed interpolation
    if (image.empty() || scale <= 1) return image.clone();

    try {
        // First, apply bicubic upscaling
        cv::Mat upscaled = bicubicUpscale(image, scale);
        
        // Edge enhancement on upscaled image
        cv::Mat edges, enhanced;
        cv::Canny(upscaled, edges, 50, 150);
        cv::dilate(edges, edges, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
        
        // Apply edge-preserving filter
        enhanced = edgePreservingFilter(upscaled);
        
        return enhanced;
    } catch (const std::exception& e) {
        Utils::logError("Exception in edge-directed interpolation: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::skinSmoothing(const cv::Mat& image, const std::vector<cv::Rect>& faceRegions, double strength) {
    if (image.empty() || faceRegions.empty()) return image.clone();

    try {
        cv::Mat result = image.clone();
        cv::Mat skinMask = createSkinMask(image);
        
        for (const auto& face : faceRegions) {
            // Ensure face region is within image bounds
            cv::Rect safeFace = face & cv::Rect(0, 0, image.cols, image.rows);
            if (safeFace.area() == 0) continue;
            
            cv::Mat faceROI = result(safeFace);
            cv::Mat faceMask = skinMask(safeFace);
            
            // Apply bilateral smoothing to face region
            cv::Mat smoothed = bilateralSkinSmoothing(faceROI, faceMask, 15);
            
            // Blend with original based on strength
            cv::addWeighted(faceROI, 1.0 - strength, smoothed, strength, 0, faceROI);
        }
        
        return result;
    } catch (const std::exception& e) {
        Utils::logError("Exception in skin smoothing: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::bilateralSkinSmoothing(const cv::Mat& image, const cv::Mat& mask, int kernelSize) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat smoothed;
        cv::bilateralFilter(image, smoothed, kernelSize, kernelSize * 2, kernelSize / 2);
        
        if (!mask.empty()) {
            cv::Mat result;
            image.copyTo(result);
            smoothed.copyTo(result, mask);
            return result;
        }
        
        return smoothed;
    } catch (const std::exception& e) {
        Utils::logError("Exception in bilateral skin smoothing: " + std::string(e.what()));
        return image.clone();
    }
}

cv::Mat EnhancementAlgorithms::createSkinMask(const cv::Mat& image) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat ycrcb, mask;
        cv::cvtColor(image, ycrcb, cv::COLOR_BGR2YCrCb);
        
        // Define skin color range in YCrCb
        cv::Scalar lower(0, 133, 77);
        cv::Scalar upper(255, 173, 127);
        
        cv::inRange(ycrcb, lower, upper, mask);
        
        // Morphological operations to clean up the mask
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
        
        return mask;
    } catch (const std::exception& e) {
        Utils::logError("Exception creating skin mask: " + std::string(e.what()));
        return cv::Mat();
    }
}

// Additional helper implementations would go here for the more complex algorithms
// like Wiener deconvolution, Richardson-Lucy, etc. These require more advanced
// mathematical implementations that would significantly increase the code size.

cv::Mat EnhancementAlgorithms::createGaussianKernel(int size, double sigma) {
    cv::Mat kernel = cv::getGaussianKernel(size, sigma, CV_32F);
    return kernel * kernel.t();
}

cv::Mat EnhancementAlgorithms::createMotionBlurKernel(int size, double angle) {
    cv::Mat kernel = cv::Mat::zeros(size, size, CV_32F);
    
    double angleRad = angle * CV_PI / 180.0;
    double cosAngle = cos(angleRad);
    double sinAngle = sin(angleRad);
    
    int center = size / 2;
    for (int i = -center; i <= center; ++i) {
        int x = center + static_cast<int>(i * cosAngle);
        int y = center + static_cast<int>(i * sinAngle);
        
        if (x >= 0 && x < size && y >= 0 && y < size) {
            kernel.at<float>(y, x) = 1.0f;
        }
    }
    
    cv::normalize(kernel, kernel, 0, 1, cv::NORM_L1);
    return kernel;
}