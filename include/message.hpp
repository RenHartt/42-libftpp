#pragma once

#include "data_buffer.hpp"

class Message {
public:
    using Type = int;

    Message(int type) : type_(type) {}

    template<typename T>
    Message& operator<<(const T& value) {
        buffer_ << value;
        return *this;
    }

    template<typename T>
    Message& operator>>(T& value) const {
        buffer_ >> value;
        return const_cast<Message&>(*this);
    }

    Type type(void) const { return type_; }

    // ——— Empaqueter en trame [type|length|payload] ———
    std::vector<uint8_t> raw() const {
        std::vector<uint8_t> frame;
        frame.reserve(8 + buffer_.size());

        appendUInt32BE(frame, static_cast<uint32_t>(type_));
        appendUInt32BE(frame, static_cast<uint32_t>(buffer_.size()));

        std::vector<uint8_t> buffer = buffer_.data();
        frame.insert(frame.end(), buffer.begin(), buffer.end());

        return frame;
    }

    // ——— Dépaqueter une trame brute en Message ———
    static Message fromRaw(const std::vector<uint8_t>& frame) {
        if (frame.size() < 8)
            throw std::runtime_error("Frame too short");

        uint32_t    netType   = readUInt32BE(frame.data());
        uint32_t    netLength = readUInt32BE(frame.data() + 4);
        std::size_t payloadSz = static_cast<size_t>(netLength);

        if (frame.size() < 8 + payloadSz)
            throw std::runtime_error("Incomplete frame");

        Message msg(static_cast<Type>(netType));
        msg.buffer_.insert(frame.data() + 8, payloadSz);
        msg.buffer_.resetReadPos();

        return msg;
    }

    static void appendUInt32BE(std::vector<uint8_t>& buf, uint32_t v) {
        buf.push_back(uint8_t((v >> 24) & 0xFF));
        buf.push_back(uint8_t((v >> 16) & 0xFF));
        buf.push_back(uint8_t((v >>  8) & 0xFF));
        buf.push_back(uint8_t((v      ) & 0xFF));
    }

    static uint32_t readUInt32BE(const uint8_t* p) {
        return (uint32_t(p[0]) << 24)
             | (uint32_t(p[1]) << 16)
             | (uint32_t(p[2]) <<  8)
             |  uint32_t(p[3]);
    }

private:
    Type type_;
    mutable DataBuffer buffer_;
};