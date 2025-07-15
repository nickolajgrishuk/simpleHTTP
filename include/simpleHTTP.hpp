#pragma once

#include <pthread.h>

#include <algorithm>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "socket.hpp"

namespace SimpleHTTP {

struct HttpHeaders {
    std::map<std::string, std::string> headers;

    void addHeader(const std::string& key, const std::string& value);
    std::string getHeader(const std::string& key) const;
    std::string toString() const;
    void clear();
};

struct HttpResponse {
    std::string body;
    std::string url;
    std::string path;
    std::string remoteAddr;
    size_t contentLength;
    int httpCode;
    HttpHeaders headers;
    std::string statusText;
    std::string protocol;

    HttpResponse();
    void clear();
};

struct UrlInfo {
    std::string protocol;
    std::string host;
    int port;
    std::string path;
    std::string query;

    UrlInfo();
    static UrlInfo parseUrl(const std::string& url);
};

class HttpClient {
    std::unique_ptr<class Socket> socket;
    std::string userAgent;
    int timeoutSeconds;

    class ThreadGuard {
        pthread_t threadId;
        bool isJoined;

    public:
        explicit ThreadGuard(const pthread_t& id);
        ~ThreadGuard();

        ThreadGuard(const ThreadGuard&) = delete;
        ThreadGuard& operator=(const ThreadGuard&) = delete;

        ThreadGuard(ThreadGuard&& other) noexcept;
        ThreadGuard& operator=(ThreadGuard&& other) noexcept;

        void join();
        void detach();
    };

public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpClient(HttpClient&& other) noexcept;
    HttpClient& operator=(HttpClient&& other) noexcept;

    HttpResponse Get(const std::string& url, const HttpHeaders& headers = HttpHeaders());

    HttpResponse Post(const std::string& url, const std::string& payload = "",
                      const std::string& contentType = "",
                      const HttpHeaders& headers = HttpHeaders());

    HttpResponse Put(const std::string& url, const std::string& payload = "",
                     const std::string& contentType = "",
                     const HttpHeaders& headers = HttpHeaders());

    HttpResponse Delete(const std::string& url, const std::string& payload = "",
                        const std::string& contentType = "",
                        const HttpHeaders& headers = HttpHeaders());

    void setTimeout(const int& seconds);
    void setUserAgent(const std::string& agent);

    std::unique_ptr<ThreadGuard> getAsync(
        const std::string& url, const HttpHeaders& headers = HttpHeaders(),
        const std::function<void(HttpResponse)>& callback = nullptr);

    std::unique_ptr<ThreadGuard> postAsync(
        const std::string& url, const std::string& payload = "",
        const std::string& contentType = "", const HttpHeaders& headers = HttpHeaders(),
        const std::function<void(HttpResponse)>& callback = nullptr);

    HttpResponse executeRequest(const std::string& method, const std::string& url,
                                const std::string& payload = "",
                                const std::string& contentType = "",
                                const HttpHeaders& headers = HttpHeaders());

    bool Download(const std::string& url, std::function<bool(const char* data, size_t size)> onChunk, const HttpHeaders& headers);

private:
    std::string buildHttpRequest(const std::string& method, const UrlInfo& urlInfo,
                                 const std::string& payload, const std::string& contentType,
                                 const HttpHeaders& headers) const;

    static HttpResponse parseHttpResponse(const std::string& rawResponse);
};

}  // namespace SimpleHTTP