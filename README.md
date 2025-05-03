# 🧵 TCP Server with Thread Pool in C++

## Overview

This project implements a **multithreaded TCP server** in C++ using a **thread pool architecture**. It efficiently handles multiple client connections concurrently using a shared pool of worker threads. The server is designed to demonstrate system-level programming concepts such as **sockets**, **concurrency**, **synchronization**, and **resource cleanup**.

This was developed as a hands-on project to strengthen my understanding of OS and networking fundamentals, and demonstrate production-grade C++ server design.

---

## 🔧 Features

- ✅ Built using C++17 and POSIX sockets  
- ✅ Efficient connection handling with a fixed-size **thread pool**  
- ✅ Graceful shutdown of threads and client sockets  
- ✅ Thread-safe client queue using `std::mutex` and `std::condition_variable`  
- ✅ Modular design using **object-oriented inheritance**  
- ✅ Logs connection lifecycle, errors, and thread activity

---

## 🛠️ Technologies Used

| Tool/Library | Purpose |
|--------------|---------|
| C++17 | Core programming language |
| POSIX Sockets | TCP connection handling |
| `std::thread`, `std::mutex`, `std::condition_variable` | Concurrency and synchronization |
| `std::atomic` | Graceful stop signaling |
| Inheritance | Extends from a base `TcpServer` class |

---

## 🚀 How It Works

### 1. **Server Startup**
- `listen_request()` sets up the socket and starts `N` worker threads.

### 2. **Client Acceptance Loop**
- `run()` accepts incoming TCP connections.
- Each connection’s `client_fd` is pushed into a thread-safe queue.

### 3. **Thread Pool Execution**
- Worker threads wait on the queue.
- When notified, a thread picks a task (client FD) and processes it using `handle_connection()`.

### 4. **Graceful Shutdown**
- `stop()` sets a stop flag, notifies all threads, joins them, and closes leftover client FDs.

---

## 📁 Directory Structure

