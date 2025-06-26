#pragma once

#include <string>
#include <functional>
#include <map>
#include <thread>
#include <mutex>

class PersistentWorker {
public:
    PersistentWorker(void) : thread([this] { this->workerLoop(); }) {}
    ~PersistentWorker(void) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    void addTask(const std::string& name, const std::function<void()>& jobToExecute) {
        std::lock_guard<std::mutex> lock(mutex);
        tasks[name] = jobToExecute;
    }

    void removeTask(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex);
        tasks.erase(name);
    }

private:
    void workerLoop(void) {
        while (true) {
            std::lock_guard<std::mutex> lock(mutex);
            for (const auto& [name, task] : tasks) {
                task();
            }
        }
    }

    std::thread thread;
    std::mutex mutex;
    std::map<std::string, std::function<void()>> tasks;
};