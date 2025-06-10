#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>
#include <algorithm>

template<typename TType>
class Pool {
public:
    Pool() noexcept
        : buffer(nullptr), free_list(nullptr), capacity(0) {}
    ~Pool() noexcept {
        for (std::size_t i = 0; i < capacity; ++i) {
            if (free_list[i] == false) {
                char* base = static_cast<char*>(buffer);
                TType* ptr = reinterpret_cast<TType*>(base + i * sizeof(TType));
                ptr->~TType();
            }
        }
        delete[] free_list;
        operator delete[](buffer);
    }
    
    class Object;

    void resize(std::size_t newCap) {
        void* newBuf   = operator new[](newCap * sizeof(TType));
        bool* newFree  = new bool[newCap];
        std::fill(newFree, newFree + newCap, true);

        try {
            for (std::size_t i = 0; i < capacity && i < newCap; ++i) {
                if (!free_list[i]) {
                    char* oldBase = static_cast<char*>(buffer);
                    TType* oldPtr = reinterpret_cast<TType*>(oldBase + i * sizeof(TType));
                    char* newBase = static_cast<char*>(newBuf);
                    TType* newPtr = reinterpret_cast<TType*>(newBase + i * sizeof(TType));
                    new (newPtr) TType(std::move(*oldPtr));
                    newFree[i] = false;
                    oldPtr->~TType();
                }
            }
            for (std::size_t i = newCap; i < capacity; ++i) {
                if (!free_list[i]) {
                    char* base = static_cast<char*>(buffer);
                    TType* ptr = reinterpret_cast<TType*>(base + i * sizeof(TType));
                    ptr->~TType();
                }
            }
        } catch(...) {
            operator delete[](newBuf);
            delete[] newFree;
            throw;
        }

        if (buffer) {
            operator delete[](buffer);
            delete[] free_list;
        }

        buffer     = newBuf;
        free_list  = newFree;
        capacity   = newCap;
    }

    template<typename... TArgs>
    Object acquire(TArgs&&... args) {
        std::size_t i = 0;
        while (i < capacity && !free_list[i]) ++i;
        if (i == capacity) throw std::bad_alloc();

        free_list[i] = false;
        char* base  = static_cast<char*>(buffer);
        TType* ptr  = reinterpret_cast<TType*>(base + i * sizeof(TType));

        try {
            new (ptr) TType(std::forward<TArgs>(args)...);
        } catch (...) {
            free_list[i] = true;
            throw;
        }

        return Object(this, i, ptr);
}

private:
    void*       buffer;
    bool*       free_list;
    std::size_t capacity;
};

template<typename TType>
class Pool<TType>::Object {
public:
    Object(Object&& other) noexcept
        : pool(other.pool), index(other.index), ptr(other.ptr), released(other.released) {
        other.released = true;
    }

    ~Object() noexcept {
        if (!released) {
            ptr->~TType();
            pool->free_list[index] = true;
            released = true;
        }
    }

    TType* operator->() { return ptr; }

private:
    friend Pool;
    Object(Pool* p, std::size_t idx, TType* pPtr) noexcept
        : pool(p), index(idx), ptr(pPtr), released(false) {}

    Pool<TType>*    pool;
    std::size_t     index;
    TType*          ptr;
    bool            released;
};


