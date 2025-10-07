#include "web_server.h"
#include <iostream>
#include <signal.h>
#include <opencv2/opencv.hpp>

// Global server instance for signal handling
WebServer* g_server = nullptr;

void signalHandler(int signal) {
    if (g_server) {
        std::cout << "\nShutting down server..." << std::endl;
        g_server->stop();
    }
    exit(0);
}

int main() {
    std::cout << "=== Face Enhancement C++ Server ===" << std::endl;
    std::cout << "OpenCV Version: " << CV_VERSION << std::endl;
    
    // Setup signal handling for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Create server configuration
    WebServer::ServerConfig config;
    config.port = 8084;
    config.webRoot = "../../web";  // Relative to cpp/build directory
    config.enableCORS = true;
    
    // Create and start server
    WebServer server(config);
    g_server = &server;
    
    if (!server.start()) {
        std::cerr << "Failed to start server!" << std::endl;
        return 1;
    }
    
    std::cout << "Server running on http://localhost:" << config.port << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Keep main thread alive
    while (server.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}