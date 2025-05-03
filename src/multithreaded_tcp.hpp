#include "Tcp.hpp" // Include the base class header
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <chrono>       // For sleep#endif 

class TCPServerThreadPool : public TcpServer {
    std::vector<std::thread> workers;
    std::queue<int> task_queue;
    std::mutex queue_mutex;

    std::condition_variable condition;
    std::atomic<bool> stop_requested{false};
    const int max_threads = std::thread::hardware_concurrency(); // Get the number of available threads



    void worker_thread() {
        while(true) {
            int client_fd = -1;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this] {return stop_requested || !task_queue.empty();});

                if(stop_requested && task_queue.empty()) {
                    log("Worker thread stopping");
                    return;
                }

                if(!task_queue.empty()) {
                    log("worker thread picked a task with client_fd: " + std::to_string(task_queue.front()));
                    // Get the client_fd from the queue
                    client_fd = task_queue.front();
                    task_queue.pop();
                } else {
                    log("Worker thread waiting for task");
                    continue; // No task available, wait for a new one
                }
            }
            // lock is released here, so we can process the task
            try{
                TcpServer::handle_connection(client_fd);
            }catch(const std::exception &e){
                log_error("Exception in worker thread: " + std::string(e.what()));
            } catch(...){
                log_error("Unknown error in worker thread");
            }
            // Close the client socket after processing
            TcpServer::close_socket(client_fd);
            log("Worker thread finished processing client_fd: " + std::to_string(client_fd));
        }
    }
public:
    TCPServerThreadPool(int port, size_t num_threads = std::thread::hardware_concurrency())
        : TcpServer(port), max_threads(num_threads) {
        if (num_threads == 0) {
            throw std::invalid_argument("Number of threads must be greater than 0");
        }
    }

    ~TCPServerThreadPool() {
        log("TCPServerThreadPool destructor called");
        // stop();
    }

    void listen_request() override {
        std::cout<<"helllll"<<std::endl;
        log("Starting TCP server with thread pool");
        if(!workers.empty()) {
            log_error("Thread pool already started");
            return;
        }
        // call the base class listen_request to listen for incoming connections
        TcpServer::listen_request();

        // start worker threads
        stop_requested = false; // Reset the stop flag
        workers.reserve(max_threads);
        log("Starting with " + std::to_string(max_threads) + " threads");
        for (size_t i = 0; i < max_threads; i++) {
            workers.emplace_back(&TCPServerThreadPool::worker_thread, this);
        } 
        log("Worker threads started");
    }

    void run() override {
        log("Running TCP server with thread pool");

        if(workers.empty()) {
            log_error("Thread pool not started");
            return;
        }
        if(listener_fd<0) {
            log_error("Thread pool already stopped");
            return;
        }
        while(true) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(listener_fd, (struct sockaddr *)&client_addr, &client_len);
            if(client_fd < 0) {
                log_error("Error accepting connection");
                continue;
            }
            log("Accepted connection from " + std::string(inet_ntoa(client_addr.sin_addr)) + ":" + std::to_string(ntohs(client_addr.sin_port)));
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                task_queue.push(client_fd);
            }
            condition.notify_one(); // Notify one worker thread
        }
        // stop();
    }

    void stop() override {
        log("Stopping TCP server with thread pool");
        if(stop_requested.exchange(true)) {
            log_error("Thread pool already stopped");
            return;
        }
        
        TcpServer::stop();
        condition.notify_all(); // Notify all worker threads to stop
        for(auto &worker : workers) {
            if(worker.joinable()) {
                worker.join();
            }
        }
        workers.clear();
        log("All worker threads stopped");
        // Clear the task queue
        std::lock_guard<std::mutex> lock(queue_mutex);
        log("Clearing task queue");
        while(!task_queue.empty()) {
            int client_fd = task_queue.front();
            task_queue.pop();
            close_socket(client_fd);
        }
        log("Task queue cleared");
        log("All client sockets closed");
        log("TCP server stopped");
    }
    // Disable copy constructor and assignment operator
    TCPServerThreadPool(const TCPServerThreadPool&) = delete;
    TCPServerThreadPool& operator=(const TCPServerThreadPool&) = delete;
};
