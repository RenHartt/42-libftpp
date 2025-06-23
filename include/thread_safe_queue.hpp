#pragma once

#include <deque>
#include <exception>
#include <iterator>
#include <mutex>
#include <stdexcept>

template<typename TType>
class ThreadSafeQueue {
private:
    std::deque<TType> deque;
    std::mutex mutex;
public:
    void push_back(const TType& newElement) {
        std::lock_guard<std::mutex> lg(mutex);
        deque.push_back(newElement);
    }

    void push_front(const TType& newElement) {
        std::lock_guard<std::mutex> lg(mutex);
        deque.push_front(newElement);
    }

    TType pop_back(void) {
        std::lock_guard<std::mutex> lg(mutex);
        if (deque.empty()) {
            throw std::runtime_error("pop on empty queue");
        }
        TType t = deque.back();
        deque.pop_back();
        return t;
    }

    TType pop_front(void) {
        std::lock_guard<std::mutex> lg(mutex);
        if (deque.empty()) {
            throw std::runtime_error("pop on empty queue");
        }
        TType t = deque.front();
        deque.pop_front();
        return t;
    }

    bool empty() { return deque.empty(); }
};