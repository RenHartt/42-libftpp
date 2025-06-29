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
    mutable std::size_t pos = 0;

    void append(const void* src, std::size_t size) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(src);
        buffer.insert(buffer.end(), ptr, ptr + size);
    }

    void read(void* dst, std::size_t size) const {
        if (pos + size > buffer.size())
            throw std::out_of_range("Out of range.");
        std::memcpy(dst, buffer.data() + pos, size);
        pos += size;
    }

public:
    DataBuffer() = default;

   template<typename T>
    DataBuffer& operator<<(const T& obj) {
        std::ostringstream oss;
        oss << obj;
        std::string s = oss.str();

        uint64_t len = s.size();
        append(&len, sizeof(len));
        append(s.data(), s.size());
    
        return *this;
    }

    template<typename T>
    DataBuffer& operator>>(T& obj) const {
        uint64_t len;
        read(&len, sizeof(len));

        std::string s;
        s.resize(len);
        read(&s[0], len);

        std::istringstream iss(s);
        iss >> obj;
        
        return const_cast<DataBuffer&>(*this);
    }

    std::size_t size(void) const { return buffer.size(); }
    const std::vector<uint8_t>& data(void) const { return buffer; }
    void resetReadPos(void) const { pos = 0; }
    void clear(void) { buffer.clear();  pos = 0; }

    void insert(const uint8_t* src, std::size_t len) {
        buffer.insert(buffer.end(), src, src + len);
    }
};