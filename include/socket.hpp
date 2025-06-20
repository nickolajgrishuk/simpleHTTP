#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleHTTP {

class Socket {
private:
    int socketFd;
    bool isConnected;

public:
    Socket();
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    bool connect(const std::string& host, const int& port);
    bool send(const std::string& data) const;
    std::string receiveChunk(size_t chunkSize = 4096) const;
    std::string receiveAll() const;
    void close();
    bool isSocketConnected() const;
    int getSocketFd() const;
};

}  // namespace SimpleHTTP