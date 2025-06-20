#include "socket.hpp"

namespace SimpleHTTP {

Socket::Socket() : socketFd(-1), isConnected(false) {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) throw std::runtime_error("Failed to create socket");
}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept 
    : socketFd(other.socketFd), isConnected(other.isConnected) {
    other.socketFd = -1;
    other.isConnected = false;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        socketFd = other.socketFd;
        isConnected = other.isConnected;
        other.socketFd = -1;
        other.isConnected = false;
    }
    return *this;
}

bool Socket::connect(const std::string& host, const int &port) {
    addrinfo hints = {}, *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    const std::string portStr = std::to_string(port);

    const int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
    if (status != 0) return false;

    for (const addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (::connect(socketFd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
            isConnected = true;
            break;
        }
    }
    
    freeaddrinfo(result);
    return isConnected;
}

bool Socket::send(const std::string &data) const {
    if (!isConnected) {
        return false;
    }
    
    size_t totalSent = 0;
    const size_t dataSize = data.size();
    
    while (totalSent < dataSize) {
        const ssize_t sent = ::send(socketFd,
                             data.c_str() + totalSent, 
                             dataSize - totalSent, 
                             0);
        if (sent == -1) {
            return false;
        }
        totalSent += sent;
    }
    
    return true;
}

std::string Socket::receiveChunk(const size_t chunkSize) const {
    if (!isConnected) {
        return "";
    }
    
    std::vector<char> buffer(chunkSize);
    const ssize_t received = recv(socketFd, buffer.data(), chunkSize, 0);
    
    if (received <= 0) {
        return "";
    }
    
    return {buffer.begin(), buffer.begin() + received};
}

std::string Socket::receiveAll() const {
    if (!isConnected) return "";

    std::string result;
    std::vector<char> buffer(4096);
    
    while (true) {
        const ssize_t received = recv(socketFd, buffer.data(), buffer.size(), 0);
        if (received <= 0) break;
        result.append(buffer.begin(), buffer.begin() + received);
    }
    
    return result;
}

void Socket::close() {
    if (socketFd != -1) {
        ::close(socketFd);
        socketFd = -1;
        isConnected = false;
    }
}

bool Socket::isSocketConnected() const {
    return isConnected;
}

int Socket::getSocketFd() const {
    return socketFd;
}

}
