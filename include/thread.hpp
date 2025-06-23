#pragma once

#include <string>
#include <functional>
#include <thread>
#include "thread_safe_iostream.hpp"

class Thread {
private:
    std::thread thread;
    std::string name;
    std::function<void()> foo;

public:
    Thread(const std::string& name, std::function<void()> functToExecute) :
    name(name), foo(functToExecute) {}
    
    ~Thread() {
        stop();
    }

    void start() {
        thread = std::thread(
            [name = this->name, foo = this->foo]() {
                threadSafeCout.setPrefix(name);
                foo();
            }
        );
    }
    void stop() {
        if (thread.joinable()) {
            thread.join();
        }
    }
};