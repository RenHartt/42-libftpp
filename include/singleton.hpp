#pragma once

#include <stdexcept>
#include <utility>

template<typename TType>
class Singleton {
private:
    inline static TType* instance_ = nullptr;
public:
    static TType* instance() { return instance_; }

    template<typename ... TArgs> 
    static void instantiate(TArgs&& ... p_args) {
        if (instance_) {
            throw std::logic_error("Instance already created");
        }
        instance_ = new TType(std::forward<TArgs>(p_args)...);
    }
};