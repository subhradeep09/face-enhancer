#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <chrono>

/**
 * Utility functions for the Face Enhancer application
 */
class Utils {
public:
    // File and directory operations
    static bool createDirectory(const std::string& path);
    static bool directoryExists(const std::string& path);
    static bool fileExists(const std::string& path);
    static std::vector<std::string> listFiles(const std::string& directory, const std::string& extension = "");
    static std::string getFileExtension(const std::string& filename);
    static std::string getBasename(const std::string& path);
    static std::string joinPath(const std::string& path1, const std::string& path2);

    // String utilities
    static std::string toLowerCase(const std::string& str);
    static std::string toUpperCase(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string trim(const std::string& str);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static bool startsWith(const std::string& str, const std::string& prefix);

    // Time and performance utilities
    static std::string getCurrentTimestamp();
    static double getElapsedTime(const std::chrono::high_resolution_clock::time_point& start);
    static void printProcessingTime(const std::string& operation, double timeMs);

    // Image validation utilities
    static bool isImageFile(const std::string& filename);
    static bool isValidImageDimensions(const cv::Mat& image, int minWidth = 64, int minHeight = 64);
    static std::string getImageInfo(const cv::Mat& image);

    // Memory and system utilities
    static size_t getMemoryUsage();
    static std::string formatFileSize(size_t bytes);
    static void printSystemInfo();

    // Progress tracking
    class ProgressBar {
    public:
        ProgressBar(int total, const std::string& prefix = "Progress");
        void update(int current);
        void finish();

    private:
        int total_;
        std::string prefix_;
        std::chrono::high_resolution_clock::time_point startTime_;
        void printBar(int current);
    };

    // Configuration utilities
    struct Config {
        std::string inputPath;
        std::string outputPath;
        bool batchMode;
        bool verbose;
        bool showPreview;
        double sharpenStrength;
        double noiseReduction;
        int superResolutionScale;
        
        static Config loadFromFile(const std::string& configPath);
        bool saveToFile(const std::string& configPath) const;
        void printConfig() const;
    };

    // Error handling
    class FaceEnhancerException : public std::exception {
    public:
        explicit FaceEnhancerException(const std::string& message) : message_(message) {}
        const char* what() const noexcept override { return message_.c_str(); }
    private:
        std::string message_;
    };

    // Logging utilities
    enum LogLevel {
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR
    };

    static void setLogLevel(LogLevel level);
    static void log(LogLevel level, const std::string& message);
    static void logDebug(const std::string& message);
    static void logInfo(const std::string& message);
    static void logWarning(const std::string& message);
    static void logError(const std::string& message);

private:
    static LogLevel currentLogLevel_;
    static std::string getLogLevelString(LogLevel level);
    static std::vector<std::string> imageExtensions_;
};

#endif // UTILS_H