#pragma once

#include <cerrno>
#include <functional>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <fcntl.h>
#include <map>
#include <unistd.h>

#include "message.hpp"

class Server {
    std::vector<pollfd> pollFds_;
    std::map<Message::Type, std::function<void(long long&, const Message&)>> actions_;
    std::map<int, std::vector<uint8_t>> pending_;
public:
    void start(const std::size_t& p_port) {
        pollfd serverFd = {::socket(AF_INET, SOCK_STREAM, 0), POLLIN, 0};
        if (serverFd.fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        int yes = 1;
        setsockopt(serverFd.fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(p_port));
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(serverFd.fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            throw std::runtime_error("Bind failed");
        }

        if (listen(serverFd.fd, SOMAXCONN) < 0) {
            throw std::runtime_error("Listen failed");
        }

        int flags = fcntl(serverFd.fd, F_GETFL, 0);
        if (flags < 0) {
            throw std::runtime_error("fcntl F_GETFL failed");
        }
        if (fcntl(serverFd.fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            throw std::runtime_error("fcntl F_SETFL failed");
        }

        pollFds_.push_back(serverFd);
    }

    void defineAction(const Message::Type& messageType, const std::function<void(long long& clientID, const Message& msg)>& action) {
        actions_[messageType] = action;
    }

    void sendTo(const Message& message, long long clientID) {
        send(static_cast<int>(clientID), message);
    }

    void sendToArray(const Message& message, std::vector<long long> clientIDs) {
        for (const auto& id : clientIDs) {
            send(static_cast<int>(id), message);
        }
    }

    void sendToAll(const Message& message) {
        for (const auto& [fd, events, revents] : pollFds_) {
            if (fd == pollFds_[0].fd) continue;
            send(static_cast<int>(fd), message);
        }
    }

    void update(void) {
        int nfds = static_cast<int>(pollFds_.size());
        int ready = ::poll(pollFds_.data(), nfds, 0);
        if (ready < 0) {
            if (errno == EINTR) return;
            throw std::runtime_error("poll failed: " + std::string(std::strerror(errno)));
        }

        for (int i = 0; i < nfds; ++i) {
            auto &pfd = pollFds_[i];
            int fd = pfd.fd;
            short re = pfd.revents;
            if (re == 0) continue;

            if (fd == pollFds_[0].fd && (re & POLLIN)) {
                int clientFd = ::accept(fd, nullptr, nullptr);
                if (clientFd >= 0) {
                    int flags = ::fcntl(clientFd, F_GETFL, 0);
                    ::fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
                    pollFds_.push_back({clientFd, POLLIN, 0});
                }
                continue;
            }

            if (re & (POLLHUP | POLLERR)) {
                ::close(fd);
                pollFds_.erase(pollFds_.begin() + i);
                --i; --nfds;
                continue;
            }

            if (re & POLLOUT) {
                auto itp = pending_.find(fd);
                if (itp != pending_.end()) {
                    const uint8_t* ptr = itp->second.data();
                    size_t left = itp->second.size();
                    ssize_t sent = ::send(fd, ptr, left, 0);
                    if (sent > 0) {
                        itp->second.erase(itp->second.begin(), itp->second.begin() + sent);
                    } else if (sent < 0 && errno != EAGAIN && errno != EINTR) {
                        throw std::runtime_error("send pending failed");
                    }
                    if (itp->second.empty()) {
                        pending_.erase(itp);
                        pfd.events &= ~POLLOUT;
                    }
                }
            }

            if (re & POLLIN) {
                uint32_t hdr[2];
                ssize_t r = ::recv(fd, hdr, sizeof(hdr), MSG_PEEK);
                if (r <= 0) continue;
                uint32_t type   = ntohl(hdr[0]);
                uint32_t length = ntohl(hdr[1]);
                std::vector<uint8_t> buffer(sizeof(hdr) + length);
                r = ::recv(fd, buffer.data(), buffer.size(), MSG_WAITALL);
                if (r <= 0) continue;
                Message msg = Message::fromRaw(buffer);
                auto it = actions_.find(type);
                if (it != actions_.end()) {
                    long long clientID = static_cast<long long>(fd);
                    it->second(clientID, msg);
                }
            }
        }
    }

private:
    void subscribeWrite(int fd) {
        for (auto &pfd : pollFds_) {
            if (pfd.fd == fd) {
                pfd.events |= POLLOUT;
                break;
            }
        }
    }

    void unsubscribeWrite(int fd) {
        for (auto &pfd : pollFds_) {
            if (pfd.fd == fd) {
                pfd.events &= ~POLLOUT;
                break;
            }
        }
    }

    void send(int fd, const Message& message) {
        std::vector<uint8_t> buf;
        auto pit = pending_.find(fd);
        if (pit != pending_.end()) {
            buf = std::move(pit->second);
            pending_.erase(pit);
        }
        auto raw = message.raw();
        buf.insert(buf.end(), raw.begin(), raw.end());

        const uint8_t* ptr = buf.data();
        std::size_t left = buf.size();

        while (left > 0) {
            ssize_t n = ::send(fd, ptr, left, 0);
            if (n > 0) {
                ptr  += n;
                left -= n;
                continue;
            }
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    pending_[fd].assign(ptr, ptr + left);
                    subscribeWrite(fd);
                    return;
                }
                throw std::runtime_error(std::string{"send() failed: "} + std::strerror(errno));
            }
        }
        unsubscribeWrite(fd);
    }
};