#include "src/multithreaded_tcp.hpp"
#include <iostream>

int main() {
    try {
        // Create an instance of the multithreaded TCP server
        TCPServerThreadPool server(8080); // Use port 8080 or any desired port

        // Start the server
        server.listen_request();

        std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;

        // Keep the server running
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}