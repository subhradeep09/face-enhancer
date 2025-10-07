#include "web_server.h"
#include "image_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <regex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

WebServer::WebServer(const ServerConfig& config) 
    : config_(config), webRoot_(config.webRoot), running_(false), serverSocket_(-1) {
    enhancer_ = std::make_unique<ImageEnhancer>();
    initializeSockets();
}

WebServer::~WebServer() {
    stop();
    cleanupSockets();
}

bool WebServer::start() {
    if (running_) {
        return false;
    }

    serverSocket_ = createServerSocket(config_.port);
    if (serverSocket_ < 0) {
        logError("Failed to create server socket");
        return false;
    }

    // Register default routes
    get("/", [this](const HttpRequest& req) { return handleRoot(req); });
    get("/status", [this](const HttpRequest& req) { return handleStatus(req); });
    post("/enhance", [this](const HttpRequest& req) { return handleEnhance(req); });

    running_ = true;
    serverThread_ = std::thread(&WebServer::serverLoop, this);

    log("Server started on http://localhost:" + std::to_string(config_.port));
    return true;
}

void WebServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    
    if (serverSocket_ >= 0) {
        closeSocket(serverSocket_);
        serverSocket_ = -1;
    }

    if (serverThread_.joinable()) {
        serverThread_.join();
    }

    log("Server stopped");
}

void WebServer::get(const std::string& path, RequestHandler handler) {
    addRoute("GET", path, handler);
}

void WebServer::post(const std::string& path, RequestHandler handler) {
    addRoute("POST", path, handler);
}

void WebServer::addRoute(const std::string& method, const std::string& path, RequestHandler handler) {
    std::lock_guard<std::mutex> lock(routesMutex_);
    routes_[method][path] = handler;
}

void WebServer::serverLoop() {
    while (running_) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket_, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (running_) {
                logError("Failed to accept client connection");
            }
            continue;
        }

        // Handle client in separate thread for concurrency
        std::thread clientThread(&WebServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void WebServer::handleClient(int clientSocket) {
    char buffer[65536];
    std::string requestData;
    
    // Read request
    while (true) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            break;
        }
        buffer[bytesRead] = '\0';
        requestData += buffer;
        
        // Check if we have complete headers
        if (requestData.find("\r\n\r\n") != std::string::npos) {
            // Check Content-Length for POST requests
            size_t contentLengthPos = requestData.find("Content-Length:");
            if (contentLengthPos != std::string::npos) {
                size_t endPos = requestData.find("\r\n", contentLengthPos);
                std::string lengthStr = requestData.substr(contentLengthPos + 15, endPos - contentLengthPos - 15);
                int contentLength = std::stoi(HttpUtils::trim(lengthStr));
                
                size_t headerEnd = requestData.find("\r\n\r\n") + 4;
                int bodyLength = requestData.length() - headerEnd;
                
                // Read remaining body if needed
                while (bodyLength < contentLength) {
                    int remainingBytes = contentLength - bodyLength;
                    int toRead = std::min(remainingBytes, (int)sizeof(buffer));
                    int bytesRead = recv(clientSocket, buffer, toRead, 0);
                    if (bytesRead <= 0) break;
                    requestData.append(buffer, bytesRead);
                    bodyLength += bytesRead;
                }
            }
            break;
        }
    }

    if (!requestData.empty()) {
        HttpRequest request = parseRequest(requestData);
        HttpResponse response = handleRequest(request);
        std::string responseStr = createResponse(response);
        
        send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
        
        if (response.isBinary && !response.binaryBody.empty()) {
            send(clientSocket, (char*)response.binaryBody.data(), response.binaryBody.size(), 0);
        }
    }

    closeSocket(clientSocket);
}

WebServer::HttpRequest WebServer::parseRequest(const std::string& requestData) {
    HttpRequest request;
    std::istringstream stream(requestData);
    std::string line;

    // Parse request line
    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        std::string version;
        lineStream >> request.method >> request.path >> version;
        
        // Parse query parameters
        size_t queryPos = request.path.find('?');
        if (queryPos != std::string::npos) {
            std::string query = request.path.substr(queryPos + 1);
            request.path = request.path.substr(0, queryPos);
            request.queryParams = parseQueryParams(query);
        }
    }

    // Parse headers
    while (std::getline(stream, line) && line != "\r") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = HttpUtils::trim(line.substr(0, colonPos));
            std::string value = HttpUtils::trim(line.substr(colonPos + 1));
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            request.headers[key] = value;
        }
    }

    // Parse body
    std::string body;
    char ch;
    while (stream.get(ch)) {
        body += ch;
    }
    request.body = body;

    return request;
}

std::string WebServer::createResponse(const HttpResponse& response) {
    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 " << response.statusCode << " " << response.statusText << "\r\n";
    
    // Add default headers
    if (config_.enableCORS) {
        responseStream << "Access-Control-Allow-Origin: *\r\n";
        responseStream << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        responseStream << "Access-Control-Allow-Headers: Content-Type\r\n";
    }
    
    // Add custom headers
    for (const auto& [key, value] : response.headers) {
        responseStream << key << ": " << value << "\r\n";
    }
    
    // Content length
    if (response.isBinary) {
        responseStream << "Content-Length: " << response.binaryBody.size() << "\r\n";
    } else {
        responseStream << "Content-Length: " << response.body.length() << "\r\n";
    }
    
    responseStream << "\r\n";
    
    if (!response.isBinary) {
        responseStream << response.body;
    }
    
    return responseStream.str();
}

WebServer::HttpResponse WebServer::handleRequest(const HttpRequest& request) {
    log(request.method + " " + request.path);

    std::lock_guard<std::mutex> lock(routesMutex_);
    
    // Check for exact route match
    auto methodRoutes = routes_.find(request.method);
    if (methodRoutes != routes_.end()) {
        auto route = methodRoutes->second.find(request.path);
        if (route != methodRoutes->second.end()) {
            return route->second(request);
        }
    }

    // Handle OPTIONS for CORS
    if (request.method == "OPTIONS") {
        HttpResponse response;
        response.statusCode = 200;
        response.headers["Access-Control-Allow-Origin"] = "*";
        response.headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS";
        response.headers["Access-Control-Allow-Headers"] = "Content-Type";
        return response;
    }

    // Try static file serving
    if (request.method == "GET") {
        return handleStaticFile(request.path);
    }

    // 404 Not Found
    HttpResponse response;
    response.statusCode = 404;
    response.statusText = "Not Found";
    response.headers["Content-Type"] = "text/html";
    response.body = "<html><body><h1>404 Not Found</h1></body></html>";
    return response;
}

WebServer::HttpResponse WebServer::handleStaticFile(const std::string& path) {
    std::string filePath = webRoot_ + path;
    if (path == "/") {
        filePath = webRoot_ + "/simple_interface.html";
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        HttpResponse response;
        response.statusCode = 404;
        response.statusText = "Not Found";
        response.headers["Content-Type"] = "text/html";
        response.body = "<html><body><h1>File Not Found</h1></body></html>";
        return response;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    HttpResponse response;
    response.statusCode = 200;
    response.headers["Content-Type"] = getMimeType(filePath);
    response.body = content;
    return response;
}

WebServer::HttpResponse WebServer::handleRoot(const HttpRequest& request) {
    return handleStaticFile("/");
}

WebServer::HttpResponse WebServer::handleStatus(const HttpRequest& request) {
    HttpResponse response;
    response.headers["Content-Type"] = "application/json";
    response.body = R"({"status": "ready", "server": "Face Enhancement C++ Server", "version": "1.0"})";
    return response;
}

WebServer::HttpResponse WebServer::handleEnhance(const HttpRequest& request) {
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        // Parse multipart form data
        std::string contentType = request.headers.at("Content-Type");
        std::string boundary = HttpUtils::extractBoundary(contentType);
        
        if (boundary.empty()) {
            HttpResponse response;
            response.statusCode = 400;
            response.headers["Content-Type"] = "application/json";
            response.body = R"({"error": "Invalid Content-Type, multipart boundary not found"})";
            return response;
        }

        // Find image data in multipart form
        std::string imageBase64;
        std::string boundaryMarker = "--" + boundary;
        
        size_t pos = 0;
        while ((pos = request.body.find(boundaryMarker, pos)) != std::string::npos) {
            size_t headerStart = pos + boundaryMarker.length();
            size_t headerEnd = request.body.find("\r\n\r\n", headerStart);
            if (headerEnd == std::string::npos) break;
            
            std::string headers = request.body.substr(headerStart, headerEnd - headerStart);
            if (headers.find("name=\"image\"") != std::string::npos) {
                size_t dataStart = headerEnd + 4;
                size_t dataEnd = request.body.find("\r\n--" + boundary, dataStart);
                if (dataEnd == std::string::npos) {
                    dataEnd = request.body.find("\n--" + boundary, dataStart);
                }
                
                if (dataEnd != std::string::npos) {
                    imageBase64 = request.body.substr(dataStart, dataEnd - dataStart);
                    // Remove data URL prefix if present
                    size_t base64Start = imageBase64.find(",");
                    if (base64Start != std::string::npos) {
                        imageBase64 = imageBase64.substr(base64Start + 1);
                    }
                    break;
                }
            }
            pos = headerEnd;
        }

        if (imageBase64.empty()) {
            HttpResponse response;
            response.statusCode = 400;
            response.headers["Content-Type"] = "application/json";
            response.body = R"({"error": "No image data found in request"})";
            return response;
        }

        // Decode base64 image
        std::vector<unsigned char> imageData = ImageUtils::base64Decode(imageBase64);
        if (imageData.empty()) {
            HttpResponse response;
            response.statusCode = 400;
            response.headers["Content-Type"] = "application/json";
            response.body = R"({"error": "Failed to decode base64 image data"})";
            return response;
        }

        // Load image using OpenCV
        cv::Mat image = cv::imdecode(imageData, cv::IMREAD_COLOR);
        if (image.empty()) {
            HttpResponse response;
            response.statusCode = 400;
            response.headers["Content-Type"] = "application/json";
            response.body = R"({"error": "Failed to decode image"})";
            return response;
        }

        // Enhance image
        EnhancementResult result = enhancer_->enhance(image);
        if (!result.success) {
            HttpResponse response;
            response.statusCode = 500;
            response.headers["Content-Type"] = "application/json";
            response.body = R"({"error": ")" + result.errorMessage + R"("})";
            return response;
        }

        // Encode enhanced image to base64
        std::vector<unsigned char> enhancedData;
        cv::imencode(".jpg", result.enhancedImage, enhancedData);
        std::string enhancedBase64 = ImageUtils::base64Encode(enhancedData);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Create response JSON
        std::ostringstream jsonResponse;
        jsonResponse << "{"
                     << "\"success\": true,"
                     << "\"enhanced_image\": \"data:image/jpeg;base64," << enhancedBase64 << "\","
                     << "\"processing_time\": " << duration.count() / 1000.0 << ","
                     << "\"faces_detected\": " << result.facesDetected << ","
                     << "\"timing\": {"
                     << "\"total\": " << result.timing.totalTime << ","
                     << "\"face_detection\": " << result.timing.faceDetectionTime << ","
                     << "\"enhancement\": " << result.timing.enhancementTime << ","
                     << "\"post_processing\": " << result.timing.postProcessingTime
                     << "}"
                     << "}";

        HttpResponse response;
        response.headers["Content-Type"] = "application/json";
        response.body = jsonResponse.str();
        return response;

    } catch (const std::exception& e) {
        logError("Enhancement error: " + std::string(e.what()));
        HttpResponse response;
        response.statusCode = 500;
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"error": "Internal server error"})";
        return response;
    }
}

std::string WebServer::getMimeType(const std::string& filename) {
    std::string extension = filename.substr(filename.find_last_of('.') + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "html" || extension == "htm") return "text/html";
    if (extension == "css") return "text/css";
    if (extension == "js") return "application/javascript";
    if (extension == "json") return "application/json";
    if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
    if (extension == "png") return "image/png";
    if (extension == "gif") return "image/gif";
    if (extension == "svg") return "image/svg+xml";
    
    return "application/octet-stream";
}

std::map<std::string, std::string> WebServer::parseQueryParams(const std::string& query) {
    std::map<std::string, std::string> params;
    std::vector<std::string> pairs = HttpUtils::split(query, "&");
    
    for (const auto& pair : pairs) {
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, equalPos));
            std::string value = urlDecode(pair.substr(equalPos + 1));
            params[key] = value;
        }
    }
    
    return params;
}

std::string WebServer::urlDecode(const std::string& str) {
    std::string decoded;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                decoded += static_cast<char>(value);
                i += 2;
            } else {
                decoded += str[i];
            }
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

bool WebServer::initializeSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true;
#endif
}

void WebServer::cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int WebServer::createServerSocket(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        return -1;
    }

    // Set socket options
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        closeSocket(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 10) < 0) {
        closeSocket(serverSocket);
        return -1;
    }

    return serverSocket;
}

void WebServer::closeSocket(int socket) {
    if (socket >= 0) {
        close(socket);
    }
}

void WebServer::log(const std::string& message) {
    std::cout << "[" << HttpUtils::getCurrentTimestamp() << "] " << message << std::endl;
}

void WebServer::logError(const std::string& message) {
    std::cerr << "[" << HttpUtils::getCurrentTimestamp() << "] ERROR: " << message << std::endl;
}

// HttpUtils implementation
namespace HttpUtils {
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm* tm = std::localtime(&time);
        
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
        return std::string(buffer);
    }

    std::string extractBoundary(const std::string& contentType) {
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos) {
            return "";
        }
        return contentType.substr(boundaryPos + 9);
    }

    std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);
        
        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        tokens.push_back(str.substr(start));
        
        return tokens;
    }

    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    std::string toLower(const std::string& str) {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower;
    }
}