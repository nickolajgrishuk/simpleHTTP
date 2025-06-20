# SimpleHTTP Library

[![Build Status](https://github.com/nickolajgrishuk/simpleHTTP/workflows/CI/badge.svg)](https://github.com/nickolajgrishuk/simpleHTTP)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A simple C++ library for making HTTP requests, built on top of Unix sockets.

## Peculiarities

- Support for HTTP methods: GET, POST, PUT, DELETE
- Chunk data retrieval to save memory
- Asynchronous requests using pthread
- RAII approach for resource management
- Support for custom headers
- Error handling
- Compatibility with C++11

## Requirements

- C++11 or higher
- Unix-like system (Linux, macOS, BSD)
- CMake 3.20 or higher
- pthread

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Basic exapmle

```cpp
#include "simpleHTTP.hpp"
#include <iostream>

using namespace SimpleHTTP;

int main() {
    HttpClient client;
    
    // Простой GET запрос
    HttpResponse response = client.Get("http://httpbin.org/get");
    
    std::cout << "HTTP Code: " << response.httpCode << std::endl;
    std::cout << "Body: " << response.body << std::endl;
    
    return 0;
}
```

### POST request

```cpp
HttpResponse response = client.Post(
    "http://httpbin.org/post",
    "{\"key\":\"value\"}",
    "application/json"
);
```

### Request with custom headers

```cpp
HttpHeaders headers;
headers.addHeader("Authorization", "Bearer token123");
headers.addHeader("X-Custom-Header", "value");

HttpResponse response = client.Get("http://api.example.com/data", headers);
```

### Asynchronous requests

```cpp
auto thread = client.getAsync(
    "http://httpbin.org/delay/2",
    HttpHeaders(),
    [](const HttpResponse& response) {
        std::cout << "Async response: " << response.httpCode << std::endl;
    }
);

// Wait exiting
thread->join();
```

## API Reference

### HttpClient

The main class for making HTTP requests.

#### Конструкторы
- `HttpClient()` - create a new HTTP client

#### Methods

##### Synchronous methods
- `HttpResponse Get(const std::string& url, const HttpHeaders& headers = HttpHeaders())`
- `HttpResponse Post(const std::string& url, const std::string& payload = "", const std::string& contentType = "", const HttpHeaders& headers = HttpHeaders())`
- `HttpResponse Put(const std::string& url, const std::string& payload = "", const std::string& contentType = "", const HttpHeaders& headers = HttpHeaders())`
- `HttpResponse Delete(const std::string& url, const std::string& payload = "", const std::string& contentType = "", const HttpHeaders& headers = HttpHeaders())`

##### Asynchronous methods
- `std::unique_ptr<ThreadGuard> getAsync(const std::string& url, const HttpHeaders& headers = HttpHeaders(), std::function<void(HttpResponse)> callback = nullptr)`
- `std::unique_ptr<ThreadGuard> postAsync(const std::string& url, const std::string& payload = "", const std::string& contentType = "", const HttpHeaders& headers = HttpHeaders(), std::function<void(HttpResponse)> callback = nullptr)`

##### Configure
- `void setTimeout(int seconds)` - setting timeout
- `void setUserAgent(const std::string& agent)` - setting User-Agent

### HttpResponse

Structure containing information about the HTTP response.

#### Fields
- `std::string body` - response body
- `std::string url` - source URL
- `std::string path` - request path
- `std::string remoteAddr` - remote server address
- `size_t contentLength` - content length
- `int httpCode` - HTTP response code
- `HttpHeaders headers` - response headers
- `std::string statusText` - status text
- `std::string protocol` - protocol


### Примеры

Complete usage examples are located in the `examples/` folder.

To run the example:
```bash
cd build
./simpleHTTP_example
```

## License

See LICENSE file for details.

## Supports

The library supports:
- HTTP/1.1
- All major HTTP methods
- Custom headers
- Asynchronous requests
- Error handling

## Limitations (TODO)

- HTTP only (no HTTPS)
- No proxy support
- No authentication support
- No cookie support
- No redirect support