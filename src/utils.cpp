#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#include <sys/utsname.h>
#endif

// Initialize static members
Utils::LogLevel Utils::currentLogLevel_ = Utils::LOG_INFO;
std::vector<std::string> Utils::imageExtensions_ = {
    ".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".webp", ".jp2"
};

// File and directory operations
bool Utils::createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception& e) {
        logError("Failed to create directory " + path + ": " + e.what());
        return false;
    }
}

bool Utils::directoryExists(const std::string& path) {
    try {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    } catch (const std::exception& e) {
        logError("Error checking directory existence: " + std::string(e.what()));
        return false;
    }
}

bool Utils::fileExists(const std::string& path) {
    try {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    } catch (const std::exception& e) {
        logError("Error checking file existence: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> Utils::listFiles(const std::string& directory, const std::string& extension) {
    std::vector<std::string> files;
    
    try {
        if (!directoryExists(directory)) {
            logWarning("Directory does not exist: " + directory);
            return files;
        }

        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                
                if (extension.empty() || endsWith(toLowerCase(filename), toLowerCase(extension))) {
                    files.push_back(filename);
                }
            }
        }
        
        std::sort(files.begin(), files.end());
    } catch (const std::exception& e) {
        logError("Error listing files in directory " + directory + ": " + e.what());
    }
    
    return files;
}

std::string Utils::getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos && dotPos < filename.length() - 1) {
        return filename.substr(dotPos);
    }
    return "";
}

std::string Utils::getBasename(const std::string& path) {
    try {
        return std::filesystem::path(path).filename().string();
    } catch (const std::exception& e) {
        logError("Error getting basename: " + std::string(e.what()));
        return path;
    }
}

std::string Utils::joinPath(const std::string& path1, const std::string& path2) {
    try {
        std::filesystem::path p1(path1);
        std::filesystem::path p2(path2);
        return (p1 / p2).string();
    } catch (const std::exception& e) {
        logError("Error joining paths: " + std::string(e.what()));
        return path1 + "/" + path2;
    }
}

// String utilities
std::string Utils::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string Utils::toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string Utils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

bool Utils::endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool Utils::startsWith(const std::string& str, const std::string& prefix) {
    if (prefix.length() > str.length()) return false;
    return str.compare(0, prefix.length(), prefix) == 0;
}

// Time and performance utilities
std::string Utils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

double Utils::getElapsedTime(const std::chrono::high_resolution_clock::time_point& start) {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000.0; // Return in milliseconds
}

void Utils::printProcessingTime(const std::string& operation, double timeMs) {
    logInfo(operation + " completed in " + std::to_string(timeMs) + " ms");
}

// Image validation utilities
bool Utils::isImageFile(const std::string& filename) {
    std::string ext = toLowerCase(getFileExtension(filename));
    return std::find(imageExtensions_.begin(), imageExtensions_.end(), ext) != imageExtensions_.end();
}

bool Utils::isValidImageDimensions(const cv::Mat& image, int minWidth, int minHeight) {
    return !image.empty() && image.cols >= minWidth && image.rows >= minHeight;
}

std::string Utils::getImageInfo(const cv::Mat& image) {
    if (image.empty()) {
        return "Empty image";
    }
    
    std::stringstream ss;
    ss << image.cols << "x" << image.rows;
    ss << ", " << image.channels() << " channels";
    ss << ", " << (image.depth() == CV_8U ? "8-bit" : 
                  image.depth() == CV_16U ? "16-bit" : 
                  image.depth() == CV_32F ? "32-bit float" : "unknown depth");
    
    return ss.str();
}

// Memory and system utilities
size_t Utils::getMemoryUsage() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert KB to bytes on Linux
    }
    return 0;
#endif
}

std::string Utils::formatFileSize(size_t bytes) {
    const char* sizes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024 && i < 4) {
        size /= 1024;
        i++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << sizes[i];
    return ss.str();
}

void Utils::printSystemInfo() {
    logInfo("=== System Information ===");
    
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    logInfo("Processor count: " + std::to_string(sysInfo.dwNumberOfProcessors));
    
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        logInfo("Total physical memory: " + formatFileSize(memInfo.ullTotalPhys));
        logInfo("Available physical memory: " + formatFileSize(memInfo.ullAvailPhys));
    }
#else
    struct utsname unameData;
    if (uname(&unameData) == 0) {
        logInfo("System: " + std::string(unameData.sysname));
        logInfo("Machine: " + std::string(unameData.machine));
        logInfo("Version: " + std::string(unameData.version));
    }
#endif

    logInfo("Current memory usage: " + formatFileSize(getMemoryUsage()));
    logInfo("========================");
}

// Progress Bar implementation
Utils::ProgressBar::ProgressBar(int total, const std::string& prefix) 
    : total_(total), prefix_(prefix), startTime_(std::chrono::high_resolution_clock::now()) {
    printBar(0);
}

void Utils::ProgressBar::update(int current) {
    printBar(current);
}

void Utils::ProgressBar::finish() {
    printBar(total_);
    std::cout << std::endl;
    
    double elapsed = getElapsedTime(startTime_);
    logInfo("Total time: " + std::to_string(elapsed) + " ms");
}

void Utils::ProgressBar::printBar(int current) {
    int barWidth = 50;
    float progress = static_cast<float>(current) / total_;
    int pos = static_cast<int>(barWidth * progress);
    
    std::cout << "\r" << prefix_ << " [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% (" << current << "/" << total_ << ")";
    std::cout.flush();
}

// Configuration utilities
Utils::Config Utils::Config::loadFromFile(const std::string& configPath) {
    Config config;
    
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            logWarning("Could not open config file: " + configPath + ". Using defaults.");
            return config;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;
            
            size_t equalPos = line.find('=');
            if (equalPos == std::string::npos) continue;
            
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));
            
            if (key == "input_path") config.inputPath = value;
            else if (key == "output_path") config.outputPath = value;
            else if (key == "batch_mode") config.batchMode = (value == "true");
            else if (key == "verbose") config.verbose = (value == "true");
            else if (key == "show_preview") config.showPreview = (value == "true");
            else if (key == "sharpen_strength") config.sharpenStrength = std::stod(value);
            else if (key == "noise_reduction") config.noiseReduction = std::stod(value);
            else if (key == "super_resolution_scale") config.superResolutionScale = std::stoi(value);
        }
        
        logInfo("Configuration loaded from: " + configPath);
    } catch (const std::exception& e) {
        logError("Error loading config: " + std::string(e.what()));
    }
    
    return config;
}

bool Utils::Config::saveToFile(const std::string& configPath) const {
    try {
        std::ofstream file(configPath);
        if (!file.is_open()) {
            logError("Could not open config file for writing: " + configPath);
            return false;
        }
        
        file << "# Face Enhancer Configuration\n";
        file << "input_path=" << inputPath << "\n";
        file << "output_path=" << outputPath << "\n";
        file << "batch_mode=" << (batchMode ? "true" : "false") << "\n";
        file << "verbose=" << (verbose ? "true" : "false") << "\n";
        file << "show_preview=" << (showPreview ? "true" : "false") << "\n";
        file << "sharpen_strength=" << sharpenStrength << "\n";
        file << "noise_reduction=" << noiseReduction << "\n";
        file << "super_resolution_scale=" << superResolutionScale << "\n";
        
        logInfo("Configuration saved to: " + configPath);
        return true;
    } catch (const std::exception& e) {
        logError("Error saving config: " + std::string(e.what()));
        return false;
    }
}

void Utils::Config::printConfig() const {
    logInfo("=== Current Configuration ===");
    logInfo("Input path: " + inputPath);
    logInfo("Output path: " + outputPath);
    logInfo("Batch mode: " + std::string(batchMode ? "enabled" : "disabled"));
    logInfo("Verbose: " + std::string(verbose ? "enabled" : "disabled"));
    logInfo("Show preview: " + std::string(showPreview ? "enabled" : "disabled"));
    logInfo("Sharpen strength: " + std::to_string(sharpenStrength));
    logInfo("Noise reduction: " + std::to_string(noiseReduction));
    logInfo("Super resolution scale: " + std::to_string(superResolutionScale));
    logInfo("============================");
}

// Logging utilities
void Utils::setLogLevel(LogLevel level) {
    currentLogLevel_ = level;
}

void Utils::log(LogLevel level, const std::string& message) {
    if (level < currentLogLevel_) return;
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = getLogLevelString(level);
    
    std::cout << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
}

void Utils::logDebug(const std::string& message) {
    log(LOG_DEBUG, message);
}

void Utils::logInfo(const std::string& message) {
    log(LOG_INFO, message);
}

void Utils::logWarning(const std::string& message) {
    log(LOG_WARNING, message);
}

void Utils::logError(const std::string& message) {
    log(LOG_ERROR, message);
}

std::string Utils::getLogLevelString(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO ";
        case LOG_WARNING: return "WARN ";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}