#include "simpleHTTP.hpp"

namespace SimpleHTTP {

// HttpHeaders implementation
void HttpHeaders::addHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

std::string HttpHeaders::getHeader(const std::string& key) const {
    const auto it = headers.find(key);
    return (it != headers.end()) ? it->second : "";
}

std::string HttpHeaders::toString() const {
    std::stringstream ss;
    for (const auto& header : headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }
    return ss.str();
}

void HttpHeaders::clear() {
    headers.clear();
}

HttpResponse::HttpResponse() : contentLength(0), httpCode(0) {}

void HttpResponse::clear() {
    body.clear();
    url.clear();
    path.clear();
    remoteAddr.clear();
    contentLength = 0;
    httpCode = 0;
    headers.clear();
    statusText.clear();
    protocol.clear();
}

UrlInfo::UrlInfo() : port(80) {}

UrlInfo UrlInfo::parseUrl(const std::string& url) {
    UrlInfo info;

    const size_t protocolEnd = url.find("://");
    if (protocolEnd != std::string::npos) {
        info.protocol = url.substr(0, protocolEnd);
        const size_t hostStart = protocolEnd + 3;
        size_t hostEnd = url.find('/', hostStart);

        if (hostEnd == std::string::npos)
            hostEnd = url.length();

        std::string hostPort = url.substr(hostStart, hostEnd - hostStart);
        const size_t portPos = hostPort.find(':');

        if (portPos != std::string::npos) {
            info.host = hostPort.substr(0, portPos);
            info.port = std::stoi(hostPort.substr(portPos + 1));
        } else {
            info.host = hostPort;
            info.port = (info.protocol == "https") ? 443 : 80;
        }

        if (hostEnd < url.length()) {
            const size_t pathStart = hostEnd;
            const size_t queryPos = url.find('?', pathStart);

            if (queryPos != std::string::npos) {
                info.path = url.substr(pathStart, queryPos - pathStart);
                info.query = url.substr(queryPos + 1);
            } else {
                info.path = url.substr(pathStart);
            }
        } else {
            info.path = "/";
        }
    }

    return info;
}

HttpClient::ThreadGuard::ThreadGuard(const pthread_t& id) : threadId(id), isJoined(false) {}

HttpClient::ThreadGuard::~ThreadGuard() {
    if (!isJoined)
        pthread_detach(threadId);
}

HttpClient::ThreadGuard::ThreadGuard(ThreadGuard&& other) noexcept
    : threadId(other.threadId), isJoined(other.isJoined) {
    other.isJoined = true;
}

HttpClient::ThreadGuard& HttpClient::ThreadGuard::operator=(ThreadGuard&& other) noexcept {
    if (this != &other) {
        if (!isJoined) {
            pthread_detach(threadId);
        }
        threadId = other.threadId;
        isJoined = other.isJoined;
        other.isJoined = true;
    }
    return *this;
}

void HttpClient::ThreadGuard::join() {
    if (!isJoined) {
        pthread_join(threadId, nullptr);
        isJoined = true;
    }
}

void HttpClient::ThreadGuard::detach() {
    if (!isJoined) {
        pthread_detach(threadId);
        isJoined = true;
    }
}

HttpClient::HttpClient() : socket(new Socket()), userAgent("SimpleHTTP/1.0"), timeoutSeconds(30) {}

HttpClient::~HttpClient() = default;

HttpClient::HttpClient(HttpClient&& other) noexcept
    : socket(std::move(other.socket)),
      userAgent(std::move(other.userAgent)),
      timeoutSeconds(other.timeoutSeconds) {}

HttpClient& HttpClient::operator=(HttpClient&& other) noexcept {
    if (this != &other) {
        socket = std::move(other.socket);
        userAgent = std::move(other.userAgent);
        timeoutSeconds = other.timeoutSeconds;
    }
    return *this;
}

HttpResponse HttpClient::Get(const std::string& url, const HttpHeaders& headers) {
    return executeRequest("GET", url, "", "", headers);
}

HttpResponse HttpClient::Post(const std::string& url, const std::string& payload,
                              const std::string& contentType, const HttpHeaders& headers) {
    return executeRequest("POST", url, payload, contentType, headers);
}

HttpResponse HttpClient::Put(const std::string& url, const std::string& payload,
                             const std::string& contentType, const HttpHeaders& headers) {
    return executeRequest("PUT", url, payload, contentType, headers);
}

HttpResponse HttpClient::Delete(const std::string& url, const std::string& payload,
                                const std::string& contentType, const HttpHeaders& headers) {
    return executeRequest("DELETE", url, payload, contentType, headers);
}

bool HttpClient::Download(const std::string& url, std::function<bool(const char* data, size_t size)> onChunk, const HttpHeaders& headers) {
    try {
        UrlInfo urlInfo = UrlInfo::parseUrl(url);
        socket.reset(new Socket());
        if (!socket->connect(urlInfo.host, urlInfo.port)) return false;

        std::string request = buildHttpRequest("GET", urlInfo, "", "", headers);
        if (!socket->send(request)) return false;

        std::string headersStr;
        while (true) {
            std::string chunk = socket->receiveChunk(1);
            if (chunk.empty()) return false;
            headersStr += chunk;
            if (headersStr.find("\r\n\r\n") != std::string::npos) break;
        }
        size_t bodyStart = headersStr.find("\r\n\r\n") + 4;

        if (bodyStart < headersStr.size()) {
            if (!onChunk(headersStr.data() + bodyStart, headersStr.size() - bodyStart)) return false;
        }

        while (true) {
            std::string chunk = socket->receiveChunk(8192);
            if (chunk.empty()) break;
            if (!onChunk(chunk.data(), chunk.size())) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

void HttpClient::setTimeout(const int& seconds) {
    timeoutSeconds = seconds;
}

void HttpClient::setUserAgent(const std::string& agent) {
    userAgent = agent;
}

HttpResponse HttpClient::executeRequest(const std::string& method, const std::string& url,
                                        const std::string& payload, const std::string& contentType,
                                        const HttpHeaders& headers) {
    HttpResponse response;
    response.url = url;

    try {
        UrlInfo urlInfo = UrlInfo::parseUrl(url);
        response.path = urlInfo.path;
        response.remoteAddr = urlInfo.host + ":" + std::to_string(urlInfo.port);

        socket.reset(new Socket());

        if (!socket->connect(urlInfo.host, urlInfo.port)) {
            response.httpCode = -1;
            return response;
        }

        std::string request = buildHttpRequest(method, urlInfo, payload, contentType, headers);

        if (!socket->send(request)) {
            response.httpCode = -1;
            return response;
        }

        std::string rawResponse;
        std::string chunk;

        while (!(chunk = socket->receiveChunk()).empty()) {
            rawResponse += chunk;
        }

        response = parseHttpResponse(rawResponse);
        response.url = url;
        response.path = urlInfo.path;
        response.remoteAddr = urlInfo.host + ":" + std::to_string(urlInfo.port);

    } catch (...) {
        response.httpCode = -1;
    }

    return response;
}

std::string HttpClient::buildHttpRequest(const std::string& method, const UrlInfo& urlInfo,
                                         const std::string& payload, const std::string& contentType,
                                         const HttpHeaders& headers) const {
    std::stringstream request;

    request << method << " " << urlInfo.path;
    if (!urlInfo.query.empty())
        request << "?" << urlInfo.query;

    request << " HTTP/1.1\r\n";

    request << "Host: " << urlInfo.host;
    if (urlInfo.port != 80 && urlInfo.port != 443)
        request << ":" << urlInfo.port;

    request << "\r\n";

    request << "User-Agent: " << userAgent << "\r\n";
    request << "Connection: close\r\n";

    for (const auto& header : headers.headers) {
        request << header.first << ": " << header.second << "\r\n";
    }

    if (!payload.empty()) {
        if (!contentType.empty()) {
            request << "Content-Type: " << contentType << "\r\n";
        } else {
            request << "Content-Type: application/x-www-form-urlencoded\r\n";
        }
        request << "Content-Length: " << payload.length() << "\r\n";
    }

    request << "\r\n";

    if (!payload.empty()) {
        request << payload;
    }

    return request.str();
}

static std::string decodeChunkedBody(const std::string& chunked) {
    std::istringstream stream(chunked);
    std::string decoded;
    while (true) {
        std::string line;
        if (!std::getline(stream, line)) break;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        size_t chunkSize = 0;
        std::istringstream(line) >> std::hex >> chunkSize;
        if (chunkSize == 0) break;
        std::string chunk(chunkSize, '\0');
        stream.read(&chunk[0], chunkSize);
        decoded += chunk;
        stream.get();
        stream.get();
    }
    return decoded;
}

HttpResponse HttpClient::parseHttpResponse(const std::string& rawResponse) {
    HttpResponse response;

    if (rawResponse.empty()) {
        response.httpCode = -1;
        return response;
    }

    size_t headerEnd = rawResponse.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        response.httpCode = -1;
        return response;
    }

    std::string headers = rawResponse.substr(0, headerEnd);
    response.body = rawResponse.substr(headerEnd + 4);

    std::istringstream headerStream(headers);
    std::string statusLine;
    std::getline(headerStream, statusLine);

    if (!statusLine.empty() && statusLine.back() == '\r') {
        statusLine.pop_back();
    }

    std::istringstream statusStream(statusLine);
    statusStream >> response.protocol >> response.httpCode;

    std::getline(statusStream, response.statusText);
    if (!response.statusText.empty() && response.statusText.front() == ' ') {
        response.statusText = response.statusText.substr(1);
    }

    std::string headerLine;
    bool isChunked = false;
    size_t contentLength = 0;
    while (std::getline(headerStream, headerLine)) {
        if (headerLine.empty() || headerLine == "\r")
            break;

        if (headerLine.back() == '\r')
            headerLine.pop_back();

        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 1);

            if (!value.empty() && value.front() == ' ')
                value = value.substr(1);

            response.headers.addHeader(key, value);

            if (key == "Content-Length") {
                try {
                    contentLength = std::stoul(value);
                    response.contentLength = contentLength;
                } catch (...) {
                    response.contentLength = 0;
                }
            }
            if (key == "Transfer-Encoding" && value.find("chunked") != std::string::npos) {
                isChunked = true;
            }
        }
    }

    if (isChunked) {
        response.body = decodeChunkedBody(response.body);
        response.contentLength = response.body.size();
    } else if (contentLength > 0 && response.body.size() > contentLength) {
        response.body = response.body.substr(0, contentLength);
    }

    return response;
}

struct AsyncRequestData {
    HttpClient* client;
    std::string method;
    std::string url;
    std::string payload;
    std::string contentType;
    HttpHeaders headers;
    std::function<void(HttpResponse)> callback;
};

void* asyncRequestThread(void* arg) {
    const auto* data = static_cast<AsyncRequestData*>(arg);

    const HttpResponse response = data->client->executeRequest(
        data->method, data->url, data->payload, data->contentType, data->headers);

    if (data->callback) {
        data->callback(response);
    }

    delete data;
    return nullptr;
}

std::unique_ptr<HttpClient::ThreadGuard> HttpClient::getAsync(
    const std::string& url, const HttpHeaders& headers,
    const std::function<void(HttpResponse)>& callback) {
    auto* data = new AsyncRequestData{this, "GET", url, "", "", headers, callback};

    pthread_t threadId;
    if (pthread_create(&threadId, nullptr, asyncRequestThread, data) != 0) {
        delete data;
        return nullptr;
    }

    return std::unique_ptr<ThreadGuard>(new ThreadGuard(threadId));
}

std::unique_ptr<HttpClient::ThreadGuard> HttpClient::postAsync(
    const std::string& url, const std::string& payload, const std::string& contentType,
    const HttpHeaders& headers, const std::function<void(HttpResponse)>& callback) {
    auto* data = new AsyncRequestData{this, "POST", url, payload, contentType, headers, callback};

    pthread_t threadId;
    if (pthread_create(&threadId, nullptr, asyncRequestThread, data) != 0) {
        delete data;
        return nullptr;
    }

    return std::unique_ptr<ThreadGuard>(new ThreadGuard(threadId));
}

}  // namespace SimpleHTTP
