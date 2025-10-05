#ifndef ENHANCEMENT_ALGORITHMS_H
#define ENHANCEMENT_ALGORITHMS_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

/**
 * Advanced image enhancement algorithms
 */
class EnhancementAlgorithms {
public:
    // Sharpening algorithms
    static cv::Mat unsharpMask(const cv::Mat& image, double strength = 1.5, double radius = 1.0, double threshold = 0.0);
    static cv::Mat laplacianSharpen(const cv::Mat& image, double strength = 0.8);
    static cv::Mat highPassSharpen(const cv::Mat& image, double strength = 1.0);
    
    // Noise reduction algorithms
    static cv::Mat bilateralFilter(const cv::Mat& image, int d = 9, double sigmaColor = 75.0, double sigmaSpace = 75.0);
    static cv::Mat nonLocalMeansDenoising(const cv::Mat& image, float h = 10.0f, int templateWindowSize = 7, int searchWindowSize = 21);
    static cv::Mat guidedFilter(const cv::Mat& image, const cv::Mat& guide, int radius = 8, double eps = 0.01);
    
    // Edge enhancement algorithms
    static cv::Mat edgePreservingFilter(const cv::Mat& image, int flags = cv::RECURS_FILTER, double sigmaS = 50.0, double sigmaR = 0.4);
    static cv::Mat detailEnhance(const cv::Mat& image, double sigmaS = 10.0, double sigmaR = 0.15);
    static cv::Mat pencilSketch(const cv::Mat& image, double sigmaS = 60.0, double sigmaR = 0.07, double shadeFactor = 0.05);
    
    // Contrast and brightness enhancement
    static cv::Mat adaptiveHistogramEqualization(const cv::Mat& image, double clipLimit = 2.0, const cv::Size& tileGridSize = cv::Size(8, 8));
    static cv::Mat gammaCorrection(const cv::Mat& image, double gamma = 1.0);
    static cv::Mat retinexSSR(const cv::Mat& image, double sigma = 100.0);
    static cv::Mat retinexMSR(const cv::Mat& image, const std::vector<double>& sigmas = {15.0, 80.0, 250.0});
    
    // Frequency domain enhancement
    static cv::Mat fourierSharpen(const cv::Mat& image, double cutoffFreq = 0.1);
    static cv::Mat butterworthHighPass(const cv::Mat& image, double cutoffFreq = 0.1, int order = 2);
    
    // Super resolution algorithms
    static cv::Mat bicubicUpscale(const cv::Mat& image, int scale = 2);
    static cv::Mat lanczosUpscale(const cv::Mat& image, int scale = 2);
    static cv::Mat edgeDirectedInterpolation(const cv::Mat& image, int scale = 2);
    
    // Skin enhancement algorithms
    static cv::Mat skinSmoothing(const cv::Mat& image, const std::vector<cv::Rect>& faceRegions, double strength = 0.5);
    static cv::Mat bilateralSkinSmoothing(const cv::Mat& image, const cv::Mat& mask, int kernelSize = 15);
    
    // Advanced deblurring algorithms
    static cv::Mat wienerDeconvolution(const cv::Mat& image, const cv::Mat& psf, double nsr = 0.01);
    static cv::Mat richardsonLucyDeconvolution(const cv::Mat& image, const cv::Mat& psf, int iterations = 20);
    static cv::Mat blindDeconvolution(const cv::Mat& image, int iterations = 30);
    
    // Utility functions
    static cv::Mat createGaussianKernel(int size, double sigma);
    static cv::Mat createMotionBlurKernel(int size, double angle);
    static cv::Mat estimateBlurKernel(const cv::Mat& image, int kernelSize = 15);
    static cv::Mat createSkinMask(const cv::Mat& image);
    
private:
    // Helper functions for complex algorithms
    static cv::Mat applyFFT(const cv::Mat& image);
    static cv::Mat applyIFFT(const cv::Mat& complexImage);
    static cv::Mat padImage(const cv::Mat& image, int padX, int padY);
    static cv::Mat cropPaddedImage(const cv::Mat& image, const cv::Size& originalSize);
    static std::vector<cv::Mat> splitChannels(const cv::Mat& image);
    static cv::Mat mergeChannels(const std::vector<cv::Mat>& channels);
};

#endif // ENHANCEMENT_ALGORITHMS_H