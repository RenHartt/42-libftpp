#pragma once

#include <cstddef>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <sstream>

class DataBuffer {
private:
    std::vector<uint8_t> buffer;
    std::size_t pos = 0;

    void append(const void* src, std::size_t size) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(src);
        buffer.insert(buffer.end(), ptr, ptr + size);
    }

    void read(void* dst, std::size_t size) {
        if (pos + size > buffer.size())
            throw std::out_of_range("Out of range.");
        std::memcpy(dst, buffer.data() + pos, size);
        pos += size;
    }

public:
    DataBuffer() = default;

   template<typename T>
    DataBuffer& operator<<(T const& obj) {
        std::ostringstream oss;
        oss << obj;
        std::string s = oss.str();

        uint64_t len = s.size();
        append(&len, sizeof(len));
        append(s.data(), s.size());
    
        return *this;
    }

    template<typename T>
    DataBuffer& operator>>(T& obj) {
        uint64_t len;
        read(&len, sizeof(len));

        std::string s;
        s.resize(len);
        read(&s[0], len);

        std::istringstream iss(s);
        iss >> obj;
        
        return *this;
    }

};