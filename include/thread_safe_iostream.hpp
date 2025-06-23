#pragma once

#include <iostream>
#include <ostream>
#include <mutex>
#include <sstream>
#include <string>

class ThreadSafeIOStream {
private:
    std::string prefix_;
    std::mutex mtx_;

    class Proxy {
    private:
        ThreadSafeIOStream& parent_;
        std::unique_lock<std::mutex> lock_;
        std::ostringstream oss_;
        bool prefixAdded_{false};
    public:
        Proxy(ThreadSafeIOStream& parent)
            : parent_(parent), lock_(parent_.mtx_) {}
        Proxy(Proxy&&) = default;
        Proxy(const Proxy&) = delete;
        Proxy& operator=(const Proxy&) = delete;


        template<typename T>
        Proxy& operator<<(const T& data) {
            if (!prefixAdded_) {
                oss_ << parent_.prefix_;
                prefixAdded_ = true;
            }
            oss_ << data;
            return *this;
        }

        Proxy& operator<<(std::ostream& (*manip)(std::ostream&)) {
            if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
                if (!prefixAdded_) {
                    oss_ << parent_.prefix_;
                    prefixAdded_ = true;
                }
                oss_ << '\n';
            } else {
                oss_ << manip;
            }
            return *this;
        }

        ~Proxy() {
            std::cout << oss_.str();
            std::cout.flush();
        }
    };

public:
    template<typename T>
    Proxy operator<<(const T& data) {
        Proxy p(*this);
        p << data;
        return p;
    }

    Proxy operator<<(std::ostream& (*manip)(std::ostream&)) {
        Proxy p(*this);
        p << manip;
        return p;
    }

    void setPrefix(const std::string& prefix) {
        std::lock_guard<std::mutex> lock(mtx_);
        prefix_ = prefix;
    }

    template<typename T>
    void prompt(const std::string& question, T& dest) {
        *this << question << std::endl;
        std::lock_guard<std::mutex> lock(mtx_);
        std::cin >> dest;
    }
};

inline thread_local ThreadSafeIOStream threadSafeCout;
