#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <functional>
#include <map>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>

#include "message.hpp"

class Client {
public:
    ~Client(void) {
        disconnect();
    }

    void connect(const std::string& address, const std::size_t& port) {
        rawBuf.clear();
        sockfd = -1;

        addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        int err = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &res);
        if (err) { throw std::runtime_error(gai_strerror(err)); }

        
        if ((sockfd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            freeaddrinfo(res);
            throw std::runtime_error("socket() failed");
        }

        if (::connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
            ::close(sockfd);
            freeaddrinfo(res);
            throw std::runtime_error("connect() failed");
        }

        freeaddrinfo(res);
    }

    void disconnect(void) {
        if (sockfd >= 0) {
            ::shutdown(sockfd, SHUT_RDWR);
            ::close(sockfd);
            sockfd = -1;
            handlers.clear();
            rawBuf.clear();
        }
    }

    void defineAction(const Message::Type& messageType, const std::function<void(const Message& msg)>& action) {
        std::lock_guard<std::mutex> lock(mutex);
        
        handlers[messageType] = action;
    }

    void send(const Message& message) {
        std::lock_guard<std::mutex> lock(mutex);
        
        std::vector<uint8_t> frame = message.raw();
        const uint8_t* ptr = frame.data();
        std::size_t toSend = frame.size();

        while (toSend) {
            ssize_t sent = ::send(sockfd, ptr, toSend, 0);
            if (sent < 0) {
                throw std::runtime_error("Client::send() failed");
            }

            ptr += sent;
            toSend -= sent;
        }
    }

    void update(void) {
        std::lock_guard<std::mutex> lock(mutex);

        int available = 0;
        if (::ioctl(sockfd, FIONREAD, &available) < 0) {
            throw std::runtime_error("Client::update(): ioctl failed");
        }
        if (available <= 0) return;

        std::vector<uint8_t> temp(available);
        ssize_t n = recv(sockfd, temp.data(), temp.size(), 0);
        if (n <= 0) {
            disconnect();
            return;
        }
        
        rawBuf.insert(rawBuf.end(), temp.begin(), temp.begin() + n);

        size_t offset = 0;
        while (true) {
            if (rawBuf.size() - offset < 8) break;

            uint32_t netType   = Message::readUInt32BE(rawBuf.data() + offset);
            uint32_t netLength = Message::readUInt32BE(rawBuf.data() + offset + 4);
            size_t   payloadSz = static_cast<size_t>(netLength);

            if (rawBuf.size() - offset < 8 + payloadSz) break;

            std::vector<uint8_t> frame(
                rawBuf.begin() + offset,
                rawBuf.begin() + offset + 8 + payloadSz
            );

            Message msg = Message::fromRaw(frame);
            handlers[netType](msg);

            offset += 8 + payloadSz;
        }

        if (offset) {
            rawBuf.erase(rawBuf.begin(), rawBuf.begin() + offset);
        }
    }

private:
    int sockfd{-1};
    std::mutex mutex;
    std::vector<uint8_t> rawBuf;
    std::map<Message::Type, std::function<void(const Message& msg)>> handlers;
};