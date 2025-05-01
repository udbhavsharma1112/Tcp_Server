#pragma once
#include "HttpRequestReader.hpp"
#include <stdexcept>
#include <string>
#include <map>
#include <cctype>

class HttpRequest
{
public:
    std::string start_line;
    std::map<std::string, std::string> headers;
    std::vector<char> body;

    HttpRequest parse(int fd)
    {
        HttpRequestReader reader(fd);

        // parsing header
        std::string header = reader.read_until("\r\n\r\n");
        parse_start_line(header);
        parse_header_content(header);
        
        // parsing body

        if (this->headers.count("transfer-encoding")) {
            if (this->headers["transfer-encoding"] == "chunked") {
                this->body = reader.chunked_read();
            }
        } else if (this->headers.count("content-length")) {
            size_t len = std::stoul(this->headers["content-length"]);
            this->body = reader.read_fixed(len);
        }

        return *this;
    }

private:
    void parse_start_line(const std::string &data)
    {
        size_t end = data.find("\r\n");
        if (end == std::string::npos)
            throw std::runtime_error("Invalid HTTP format");
        this->start_line = data.substr(0, end);
    }

    void parse_header_content(const std::string &data){
        size_t start = data.find("\r\n")+2;

        while(start < data.size()){
            size_t end = data.find("\r\n",start);
            if (end == std::string::npos) break;
            std::string line = data.substr(start, end - start);
            if (line.empty()) break; // End of headers
            size_t colon = line.find(":");
            if (colon == std::string::npos) continue;

            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon+1);

            // Normalize header key (lowercase)
            std::transform(key.begin(), key.end(), key.begin(),
                         [](unsigned char c) { return std::tolower(c); });
            
            // Trim whitespace
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));

            this->headers[key] = value;
            start = end + 2;

        }
    }
};