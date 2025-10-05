#include "image_processor.h"
#include "utils.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <filesystem>
#include <algorithm>

// Initialize static member
std::vector<std::string> ImageProcessor::supportedFormats_ = {
    ".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".webp", ".jp2"
};

cv::Mat ImageProcessor::loadImage(const std::string& path) {
    try {
        cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
        
        if (image.empty()) {
            Utils::logError("Failed to load image: " + path);
            return cv::Mat();
        }
        
        Utils::logDebug("Loaded image: " + path + " (" + std::to_string(image.cols) + "x" + 
                       std::to_string(image.rows) + ", " + std::to_string(image.channels()) + " channels)");
        
        return image;
    } catch (const std::exception& e) {
        Utils::logError("Exception loading image " + path + ": " + e.what());
        return cv::Mat();
    }
}

bool ImageProcessor::saveImage(const cv::Mat& image, const std::string& path, int quality) {
    if (image.empty()) {
        Utils::logError("Cannot save empty image to: " + path);
        return false;
    }

    try {
        std::vector<int> compressionParams;
        std::string ext = Utils::toLowerCase(Utils::getFileExtension(path));
        
        if (ext == ".jpg" || ext == ".jpeg") {
            compressionParams.push_back(cv::IMWRITE_JPEG_QUALITY);
            compressionParams.push_back(quality);
        } else if (ext == ".png") {
            compressionParams.push_back(cv::IMWRITE_PNG_COMPRESSION);
            compressionParams.push_back(9 - (quality / 11)); // Convert to PNG compression level
        } else if (ext == ".webp") {
            compressionParams.push_back(cv::IMWRITE_WEBP_QUALITY);
            compressionParams.push_back(quality);
        }

        bool result = cv::imwrite(path, image, compressionParams);
        
        if (result) {
            Utils::logDebug("Saved image: " + path);
        } else {
            Utils::logError("Failed to save image: " + path);
        }
        
        return result;
    } catch (const std::exception& e) {
        Utils::logError("Exception saving image " + path + ": " + e.what());
        return false;
    }
}

cv::Mat ImageProcessor::resizeImage(const cv::Mat& image, int width, int height, int interpolation) {
    if (image.empty()) {
        Utils::logError("Cannot resize empty image");
        return cv::Mat();
    }

    try {
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(width, height), 0, 0, interpolation);
        return resized;
    } catch (const std::exception& e) {
        Utils::logError("Exception resizing image: " + std::string(e.what()));
        return cv::Mat();
    }
}

cv::Mat ImageProcessor::resizeImageProportional(const cv::Mat& image, double scaleFactor, int interpolation) {
    if (image.empty()) {
        Utils::logError("Cannot resize empty image");
        return cv::Mat();
    }

    try {
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(), scaleFactor, scaleFactor, interpolation);
        return resized;
    } catch (const std::exception& e) {
        Utils::logError("Exception resizing image proportionally: " + std::string(e.what()));
        return cv::Mat();
    }
}

cv::Mat ImageProcessor::cropImage(const cv::Mat& image, const cv::Rect& roi) {
    if (image.empty()) {
        Utils::logError("Cannot crop empty image");
        return cv::Mat();
    }

    try {
        // Ensure ROI is within image bounds
        cv::Rect safeRoi = roi & cv::Rect(0, 0, image.cols, image.rows);
        
        if (safeRoi.area() == 0) {
            Utils::logError("Invalid crop region");
            return cv::Mat();
        }

        return image(safeRoi).clone();
    } catch (const std::exception& e) {
        Utils::logError("Exception cropping image: " + std::string(e.what()));
        return cv::Mat();
    }
}

double ImageProcessor::calculateSharpness(const cv::Mat& image) {
    if (image.empty()) return 0.0;

    try {
        cv::Mat gray;
        if (image.channels() > 1) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image.clone();
        }

        cv::Mat laplacian;
        cv::Laplacian(gray, laplacian, CV_64F);
        
        cv::Scalar mean, stddev;
        cv::meanStdDev(laplacian, mean, stddev);
        
        return stddev.val[0] * stddev.val[0]; // Variance of Laplacian
    } catch (const std::exception& e) {
        Utils::logError("Exception calculating sharpness: " + std::string(e.what()));
        return 0.0;
    }
}

double ImageProcessor::calculateContrast(const cv::Mat& image) {
    if (image.empty()) return 0.0;

    try {
        cv::Mat gray;
        if (image.channels() > 1) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image.clone();
        }

        cv::Scalar mean, stddev;
        cv::meanStdDev(gray, mean, stddev);
        
        return stddev.val[0]; // Standard deviation as contrast measure
    } catch (const std::exception& e) {
        Utils::logError("Exception calculating contrast: " + std::string(e.what()));
        return 0.0;
    }
}

double ImageProcessor::calculateBrightness(const cv::Mat& image) {
    if (image.empty()) return 0.0;

    try {
        cv::Mat gray;
        if (image.channels() > 1) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image.clone();
        }

        cv::Scalar mean = cv::mean(gray);
        return mean.val[0];
    } catch (const std::exception& e) {
        Utils::logError("Exception calculating brightness: " + std::string(e.what()));
        return 0.0;
    }
}

double ImageProcessor::calculatePSNR(const cv::Mat& original, const cv::Mat& enhanced) {
    if (original.empty() || enhanced.empty()) return 0.0;
    if (original.size() != enhanced.size()) return 0.0;

    try {
        cv::Mat diff;
        cv::absdiff(original, enhanced, diff);
        diff.convertTo(diff, CV_32F);
        diff = diff.mul(diff);

        cv::Scalar mse = cv::mean(diff);
        double mseValue = (mse.val[0] + mse.val[1] + mse.val[2]) / 3.0;
        
        if (mseValue < 1e-10) return 100.0; // Perfect match
        
        return 10.0 * log10((255.0 * 255.0) / mseValue);
    } catch (const std::exception& e) {
        Utils::logError("Exception calculating PSNR: " + std::string(e.what()));
        return 0.0;
    }
}

double ImageProcessor::calculateSSIM(const cv::Mat& original, const cv::Mat& enhanced) {
    if (original.empty() || enhanced.empty()) return 0.0;
    if (original.size() != enhanced.size()) return 0.0;

    try {
        cv::Mat img1, img2;
        original.convertTo(img1, CV_32F);
        enhanced.convertTo(img2, CV_32F);

        cv::Mat img1_sq, img2_sq, img1_img2;
        cv::multiply(img1, img1, img1_sq);
        cv::multiply(img2, img2, img2_sq);
        cv::multiply(img1, img2, img1_img2);

        cv::Mat mu1, mu2;
        cv::GaussianBlur(img1, mu1, cv::Size(11, 11), 1.5);
        cv::GaussianBlur(img2, mu2, cv::Size(11, 11), 1.5);

        cv::Mat mu1_sq, mu2_sq, mu1_mu2;
        cv::multiply(mu1, mu1, mu1_sq);
        cv::multiply(mu2, mu2, mu2_sq);
        cv::multiply(mu1, mu2, mu1_mu2);

        cv::Mat sigma1_sq, sigma2_sq, sigma12;
        cv::GaussianBlur(img1_sq, sigma1_sq, cv::Size(11, 11), 1.5);
        cv::GaussianBlur(img2_sq, sigma2_sq, cv::Size(11, 11), 1.5);
        cv::GaussianBlur(img1_img2, sigma12, cv::Size(11, 11), 1.5);

        sigma1_sq -= mu1_sq;
        sigma2_sq -= mu2_sq;
        sigma12 -= mu1_mu2;

        const double C1 = 6.5025, C2 = 58.5225;
        cv::Mat numerator1 = 2 * mu1_mu2 + C1;
        cv::Mat numerator2 = 2 * sigma12 + C2;
        cv::Mat denominator1 = mu1_sq + mu2_sq + C1;
        cv::Mat denominator2 = sigma1_sq + sigma2_sq + C2;

        cv::Mat ssim_map;
        cv::divide(numerator1.mul(numerator2), denominator1.mul(denominator2), ssim_map);

        cv::Scalar ssim_mean = cv::mean(ssim_map);
        return (ssim_mean.val[0] + ssim_mean.val[1] + ssim_mean.val[2]) / 3.0;
    } catch (const std::exception& e) {
        Utils::logError("Exception calculating SSIM: " + std::string(e.what()));
        return 0.0;
    }
}

std::string ImageProcessor::getImageFormat(const std::string& filename) {
    return Utils::toLowerCase(Utils::getFileExtension(filename));
}

bool ImageProcessor::isValidImageFile(const std::string& filename) {
    return isFormatSupported(getImageFormat(filename));
}

std::vector<std::string> ImageProcessor::getImagesInDirectory(const std::string& directory) {
    std::vector<std::string> imageFiles;
    
    try {
        if (!Utils::directoryExists(directory)) {
            Utils::logError("Directory does not exist: " + directory);
            return imageFiles;
        }

        std::vector<std::string> allFiles = Utils::listFiles(directory);
        for (const auto& file : allFiles) {
            if (isValidImageFile(file)) {
                imageFiles.push_back(file);
            }
        }
    } catch (const std::exception& e) {
        Utils::logError("Exception listing images in directory: " + std::string(e.what()));
    }
    
    return imageFiles;
}

cv::Mat ImageProcessor::convertToGrayscale(const cv::Mat& image) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat gray;
        if (image.channels() == 1) {
            gray = image.clone();
        } else {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        }
        return gray;
    } catch (const std::exception& e) {
        Utils::logError("Exception converting to grayscale: " + std::string(e.what()));
        return cv::Mat();
    }
}

cv::Mat ImageProcessor::convertToRGB(const cv::Mat& image) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat rgb;
        if (image.channels() == 3) {
            cv::cvtColor(image, rgb, cv::COLOR_BGR2RGB);
        } else {
            rgb = image.clone();
        }
        return rgb;
    } catch (const std::exception& e) {
        Utils::logError("Exception converting to RGB: " + std::string(e.what()));
        return cv::Mat();
    }
}

cv::Mat ImageProcessor::normalizeImage(const cv::Mat& image) {
    if (image.empty()) return cv::Mat();

    try {
        cv::Mat normalized;
        cv::normalize(image, normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
        return normalized;
    } catch (const std::exception& e) {
        Utils::logError("Exception normalizing image: " + std::string(e.what()));
        return cv::Mat();
    }
}

void ImageProcessor::displayImage(const cv::Mat& image, const std::string& windowName, int waitKey) {
    if (image.empty()) {
        Utils::logError("Cannot display empty image");
        return;
    }

    try {
        cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
        cv::imshow(windowName, image);
        
        if (waitKey >= 0) {
            cv::waitKey(waitKey);
            cv::destroyWindow(windowName);
        }
    } catch (const std::exception& e) {
        Utils::logError("Exception displaying image: " + std::string(e.what()));
    }
}

void ImageProcessor::showImageComparison(const cv::Mat& original, const cv::Mat& enhanced, const std::string& title) {
    if (original.empty() || enhanced.empty()) {
        Utils::logError("Cannot compare empty images");
        return;
    }

    try {
        cv::Mat comparison;
        
        // Resize images to same height for comparison
        int height = std::min(original.rows, enhanced.rows);
        cv::Mat orig_resized, enh_resized;
        
        double orig_ratio = static_cast<double>(height) / original.rows;
        double enh_ratio = static_cast<double>(height) / enhanced.rows;
        
        cv::resize(original, orig_resized, cv::Size(), orig_ratio, orig_ratio);
        cv::resize(enhanced, enh_resized, cv::Size(), enh_ratio, enh_ratio);
        
        // Concatenate horizontally
        cv::hconcat(orig_resized, enh_resized, comparison);
        
        // Add labels
        cv::putText(comparison, "Original", cv::Point(10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        cv::putText(comparison, "Enhanced", cv::Point(orig_resized.cols + 10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        
        displayImage(comparison, title);
    } catch (const std::exception& e) {
        Utils::logError("Exception showing image comparison: " + std::string(e.what()));
    }
}

bool ImageProcessor::isFormatSupported(const std::string& extension) {
    return std::find(supportedFormats_.begin(), supportedFormats_.end(), extension) != supportedFormats_.end();
}