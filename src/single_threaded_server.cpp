// main.cpp (Example File Name)
#include "Tcp.hpp"
#include <csignal>
#include <iostream>
#include <cstring> // For memset in signal handler setup

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int port = 3000;
    try {
        port = std::stoi(argv[1]);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid port number: " << argv[1] << std::endl;
        return 1;
    }
    // int port = std::stoi(argv[1]);
    TcpServer server(port);

    // Set up signal handling to close the server gracefully
    std::signal(SIGINT, [](int signum) {
        std::cout << "\n[INFO]: Shutting down server..." << std::endl;
        exit(0);
    });

    try {
        server.listen_request();
        server.run();
    } catch (const std::exception &e) {
        std::cerr << "[ERROR]: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}