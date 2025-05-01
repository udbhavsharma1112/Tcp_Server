#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <system_error>
#include <cstring>      // Added for strerror()
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../utility/HttpRequest.hpp"

class TcpServer{
    int listener_fd;
    int port;
    void close_socket(int fd){
        if(fd>=0){
            close(fd);
        }
    }
public:
    TcpServer(int port): listener_fd(-1), port(port){}
    ~TcpServer(){
        close_socket(listener_fd);
    }
    
    void listen_request(){
        listener_fd = socket(AF_INET, SOCK_STREAM, 0);
        //AF_INET means IPv4
        //SOCK_STREAM means TCP
        if (listener_fd < 0) {
            throw std::runtime_error("[ERROR]: error while creating socket");
        }

        // set socket options
        int opt = 1; // enable option
        // SOL_SOCKET is the level at which the option is defined
        // SO_REUSEADDR allows the socket to be bound to an address that is already in use
        // This is useful for restarting the server without waiting for the socket to be released
        if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            throw std::runtime_error("[ERROR]: error while setting socket options");
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET; // IPv4
        server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any address
        server_addr.sin_port = htons(port); // Port number

        // bind the socket to the address and port
        if(bind(listener_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0){
            throw std::runtime_error("[ERROR]: error while binding socket");
        }

        // listen for incoming connections
        if(listen(listener_fd, SOMAXCONN)<0){ // SOMAXCONN is the maximum number of pending connections
            throw std::runtime_error("[ERROR]: error while listening socket");
        }

        std::cout << "[INFO]: Listening on port " << port << std::endl;
    }
    
    int accept_request(){
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(listener_fd, (struct sockaddr *)&client_addr, &addr_len);
        if(client_fd < 0){
            throw std::runtime_error("[ERROR]: error while accepting connection");
        }
        std::cout << "[INFO]: Accepted connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
        return client_fd;
    }

    void run(){
        while(true){
            int client_fd = -1;
            try{
                client_fd = accept_request();
                handle_connection(client_fd); 
                close_socket(client_fd);
            } catch(const std::exception &e){
                std::cerr << "[ERROR]: " << e.what() << std::endl;
                handle_request(client_fd, e.what());
                close_socket(client_fd);
            } catch(...){
                std::cerr << "[ERROR]: Unknown error" << std::endl;
                handle_request(client_fd, "Unknown error");
                close_socket(client_fd);
            }
        }
    }

private:
    void handle_connection(int client_fd){
        try {

            HttpRequest request;
            request.parse(client_fd);

            std::vector<char> body = request.body;
            std::string headers = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + std::to_string(body.size()) + "\r\n"
                "Connection: close\r\n\r\n";
            
            // 3. Send all data safely
            if (!send_all(client_fd, headers.data(), headers.size()) ||
                !send_all(client_fd, body.data(), body.size())) {
                throw std::runtime_error("[ERROR]: Failed to send complete response\n");
            }

        } catch(const std::exception &e) {
            std::cerr << "Exception : " << e.what() << "\n" ;
        }
    }

    bool send_all(int client_fd, const char *data, size_t size) {
        size_t total_sent = 0;
        while (total_sent < size) {
            ssize_t sent = send(client_fd, data + total_sent, size - total_sent, 0);
            if (sent < 0) {
                std::cerr << "[ERROR]: Failed to send data: " << strerror(errno) << std::endl;
                return false;
            }
            total_sent += sent;
        }
        return true;
    }
    void handle_error(int client_fd, const std::string &error_message){
        std::cerr << "[ERROR]: " << error_message << std::endl;
        std::string response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: " + std::to_string(error_message.size()) + "\r\n\r\n" + error_message;
        send(client_fd, response.c_str(), response.size(), 0);
    }
    void handle_request(int client_fd, const std::string &error_message){
        std::cerr << "[ERROR]: " << error_message << std::endl;
        std::string response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: " + std::to_string(error_message.size()) + "\r\n\r\n" + error_message;
        send(client_fd, response.c_str(), response.size(), 0);
    }

};


