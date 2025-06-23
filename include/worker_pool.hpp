// worker_pool.hpp
#pragma once

#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "thread_safe_queue.hpp"

class IJob {
public:
    virtual ~IJob() = default;
    virtual void run() = 0;
};

class FunctionJob : public IJob {
public:
    explicit FunctionJob(std::function<void()> f) : func_(std::move(f)) {}
    void run() override { func_(); }
private:
    std::function<void()> func_;
};

class WorkerPool {
public:
    explicit WorkerPool(size_t numThreads) : stopped_(false) {
        for (size_t i = 0; i < numThreads; ++i)
            workers_.emplace_back(&WorkerPool::workerLoop, this);
    }

    ~WorkerPool() {
        shutdown();
    }

    void addJob(IJob* job) {
        queue_.push_back(job);
        cv_.notify_one();
    }

    void addJob(std::function<void()> func) {
        addJob(new FunctionJob(std::move(func)));
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) return;
            stopped_ = true;
        }
        cv_.notify_all();
        for (auto &t : workers_)
            if (t.joinable()) t.join();
    }

private:
    void workerLoop() {
        while (true) {
            IJob* job = nullptr;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [&]{ return stopped_ || !queue_.empty(); });
                if (stopped_ && queue_.empty())
                    return;
                job = queue_.pop_front();
            }
            try {
                job->run();
            } catch (...) {}
            delete job;
        }
    }

    ThreadSafeQueue<IJob*>       queue_;
    std::vector<std::thread>     workers_;
    std::mutex                   mutex_;
    std::condition_variable      cv_;
    bool                         stopped_;
};
