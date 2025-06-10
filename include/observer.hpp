#pragma once

#include <functional>
#include <map>
#include <vector>

template<typename TEvent>
class Observer {
private:
    std::map<TEvent, std::vector<std::function<void()>>> registry;
public:
    void subscribe(const TEvent& event, const std::function<void()>& lambda) {
        registry[event].push_back(lambda);
    }

    void notify(const TEvent& event) {
        for (const auto& lambda : registry[event]) {
            lambda();
        }
    }
};