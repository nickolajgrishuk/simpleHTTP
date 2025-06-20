#pragma once

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <memory>
#include <cstring>
#include <stdexcept>
#include <iostream>

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

    Socket(Socket &&other) noexcept;
    Socket& operator=(Socket &&other) noexcept;

    bool connect(const std::string &host, const int &port);
    bool send(const std::string &data) const;
    std::string receiveChunk(size_t chunkSize = 4096) const;
    std::string receiveAll() const;
    void close();
    bool isSocketConnected() const;
    int getSocketFd() const;
};

}