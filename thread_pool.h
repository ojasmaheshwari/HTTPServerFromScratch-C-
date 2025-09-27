#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    // Constructor: creates a thread pool with a given number of threads
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());

    // Destructor: stops the thread pool
    ~ThreadPool();

    // Enqueue a new task into the pool
    void enqueue(std::function<void()> task);

private:
    std::vector<std::thread> threads_;               // Worker threads
    std::queue<std::function<void()>> tasks_;        // Task queue

    std::mutex queue_mutex_;                         // Synchronization
    std::condition_variable cv_;                     // Condition variable
    bool stop_ = false;                              // Stop flag
};

