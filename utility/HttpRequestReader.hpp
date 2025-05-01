#pragma once
#include <cstddef>
#include <vector>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <sys/uio.h> // for readv
#include <algorithm>

class HttpRequestReader
{
    int fd; // request socket
    std::vector<char> buffer_;
    size_t pos_ = 0;
    size_t bufferlen_ = 0;                           // how many bytes read at a time
    static const size_t DEFAULT_BUFSIZE = 16 * 1024; // 16KB buffer

public:
    HttpRequestReader(int fd, size_t bufferSize = DEFAULT_BUFSIZE) : fd(fd), buffer_(bufferSize) {}

    std::string read_until(std::string &delimiter) //
    {
        std::string request;
        while (true)
        {

            // refill buffer
            if (pos_ >= bufferlen_)
            {
                refill_buffer(); // reading remaining request
                if (bufferlen_ == 0)
                { // reading complete
                    break;
                }
            }

            size_t remaining = bufferlen_ - pos_;
            const char *bufferPtr = buffer_.data() + pos_;
            auto it = std::search(bufferPtr, bufferPtr + remaining, delimiter.begin(), delimiter.end());
            if (it != bufferPtr + remaining)
            {
                // found delimiter
                size_t len = it - bufferPtr + delimiter.size();
                request.append(bufferlen_, len);
                pos_ += len;
                return request;
            }

            // append partial
            request.append(bufferPtr, remaining);
            pos_ = bufferlen_; // repeat refill
        }
        return request;
    }

    std::vector<char> read_fixed(size_t length)
    {
        std::vector<char> request;

        while (request.size() < length)
        {

            if (pos_ > bufferlen_)
            {
                refill_buffer();
                if (bufferlen_ == 0)
                {
                    break;
                }
            }

            size_t remaining = bufferlen_ - pos_;
            size_t neededSize = length - request.size();
            size_t to_copy = std::min(remaining, neededSize);

            request.insert(request.end(),
                           buffer_.begin() + pos_,
                           buffer_.begin() + pos_ + to_copy);
            pos_ += to_copy;
        }
        return request;
    }

private:
    void refill_buffer()
    {
        pos_ = 0;
        ssize_t n = read(fd, buffer_.data(), buffer_.size());
        if (n < 0)
        {
            throw std::runtime_error("[error]: error while reading request");
        }
        bufferlen_ = n;
    }
};