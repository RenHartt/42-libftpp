#pragma once

#include <functional>

#include "message.hpp"

class Server {
public:
    void start(const std::size_t& p_port);
    void defineAction(const Message::Type& messageType, const std::function<void(long long& clientID, const Message& msg)>& action);
    void sendTo(const Message& message, long long clientID);
    void sendToArray(const Message& message, std::vector<long long> clientIDs);
    void sendToAll(const Message& message);
    void update(void);

private:
};