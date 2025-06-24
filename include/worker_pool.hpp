#pragma once

#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "thread_safe_queue.hpp"

class WorkerPool {
public:
    class IJob {
    public:
        IJob(std::function<void()> f) : func_(std::move(f)) {}
        void run() { func_(); }
    private:
        std::function<void()> func_;
    };

    WorkerPool(std::size_t numThreads) : stopped_(false) {
        for (std::size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back(&WorkerPool::workerLoop, this);
        }
    }

    ~WorkerPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) return;
            stopped_ = true;
        }
        cv_.notify_all();
        for (auto &t : workers_) {
            if (t.joinable()) t.join();
        }
    }

    void addJob(std::function<void()> func) {
        queue_.push_back(IJob(std::move(func)));
        cv_.notify_one();
    }

private:
    void workerLoop() {
        while (true) {
            IJob job([]{});
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [&]{ return stopped_ || !queue_.empty(); });
                if (stopped_ && queue_.empty()) return;
                job = queue_.pop_front();
            }
            try {
                job.run();
            } catch (...) {}
        }
    }

    ThreadSafeQueue<IJob>       queue_;
    std::vector<std::thread>    workers_;
    std::mutex                  mutex_;
    std::condition_variable     cv_;
    bool                        stopped_;
};
