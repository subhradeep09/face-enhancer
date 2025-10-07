#pragma once

#include "image_enhancer.h"
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>

/**
 * HTTP Web Server for Face Enhancement
 * Provides REST API for image enhancement operations
 */
class WebServer {
public:
    struct ServerConfig {
        int port = 8084;
        std::string webRoot = "../web";
        bool enableCORS = true;
        int maxRequestSize = 50 * 1024 * 1024; // 50MB
    };

    struct HttpRequest {
        std::string method;
        std::string path;
        std::string body;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> queryParams;
        std::vector<unsigned char> binaryBody;
    };

    struct HttpResponse {
        int statusCode = 200;
        std::string statusText = "OK";
        std::map<std::string, std::string> headers;
        std::string body;
        std::vector<unsigned char> binaryBody;
        bool isBinary = false;
    };

    using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

    WebServer(const ServerConfig& config = ServerConfig{});
    ~WebServer();

    // Server control
    bool start();
    void stop();
    bool isRunning() const { return running_; }

    // Route registration
    void get(const std::string& path, RequestHandler handler);
    void post(const std::string& path, RequestHandler handler);
    void addRoute(const std::string& method, const std::string& path, RequestHandler handler);

    // Static file serving
    void setStaticFileRoot(const std::string& root) { webRoot_ = root; }

private:
    ServerConfig config_;
    std::string webRoot_;
    bool running_;
    std::thread serverThread_;
    int serverSocket_;
    std::mutex routesMutex_;
    
    std::map<std::string, std::map<std::string, RequestHandler>> routes_;
    
    // Image enhancer instance
    std::unique_ptr<ImageEnhancer> enhancer_;

    // Server implementation
    void serverLoop();
    void handleClient(int clientSocket);
    
    // HTTP parsing
    HttpRequest parseRequest(const std::string& requestData);
    std::string createResponse(const HttpResponse& response);
    
    // Route handling
    HttpResponse handleRequest(const HttpRequest& request);
    HttpResponse handleStaticFile(const std::string& path);
    
    // Built-in handlers
    HttpResponse handleRoot(const HttpRequest& request);
    HttpResponse handleStatus(const HttpRequest& request);
    HttpResponse handleEnhance(const HttpRequest& request);
    
    // Utility functions
    std::string getMimeType(const std::string& filename);
    std::string urlDecode(const std::string& str);
    std::map<std::string, std::string> parseQueryParams(const std::string& query);
    std::map<std::string, std::string> parseFormData(const std::string& body, const std::string& boundary);
    
    // Cross-platform socket functions
    bool initializeSockets();
    void cleanupSockets();
    int createServerSocket(int port);
    void closeSocket(int socket);
    
    // Logging
    void log(const std::string& message);
    void logError(const std::string& message);
};

/**
 * Utility functions for HTTP server
 */
namespace HttpUtils {
    std::string getCurrentTimestamp();
    std::string extractBoundary(const std::string& contentType);
    std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    std::string trim(const std::string& str);
    std::string toLower(const std::string& str);
}