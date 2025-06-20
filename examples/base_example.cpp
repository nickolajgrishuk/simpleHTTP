#include <iostream>
#include <thread>

#include "simpleHTTP.hpp"

using namespace SimpleHTTP;

void printResponse(const HttpResponse& response, const std::string& method) {
    std::cout << "\n=== " << method << " Response ===" << std::endl;
    std::cout << "URL: " << response.url << std::endl;
    std::cout << "Path: " << response.path << std::endl;
    std::cout << "Remote Address: " << response.remoteAddr << std::endl;
    std::cout << "HTTP Code: " << response.httpCode << std::endl;
    std::cout << "Status: " << response.statusText << std::endl;
    std::cout << "Protocol: " << response.protocol << std::endl;
    std::cout << "Content Length: " << response.contentLength << std::endl;

    std::cout << "\nHeaders:" << std::endl;
    for (const auto& header : response.headers.headers) {
        std::cout << "  " << header.first << ": " << header.second << std::endl;
    }

    std::cout << "\nBody (first 500 chars):" << std::endl;
    const std::string bodyPreview = response.body.substr(0, 500);
    std::cout << bodyPreview;
    if (response.body.length() > 500) {
        std::cout << "... (truncated)";
    }
    std::cout << std::endl;
    std::cout << "================================\n" << std::endl;
}

void asyncCallback(const HttpResponse& response) {
    std::cout << "Async callback received response with code: " << response.httpCode << std::endl;
    std::cout << "Body length: " << response.body.length() << " bytes" << std::endl;
}

int main() {
    std::cout << "SimpleHTTP Library Example" << std::endl;
    std::cout << "==========================" << std::endl;

    HttpClient client;

    // Настройка клиента
    client.setUserAgent("SimpleHTTP-Example/1.0");
    client.setTimeout(10);

    // Пример 1: GET запрос
    std::cout << "Making GET request to httpbin.org..." << std::endl;
    HttpResponse getResponse = client.Get("http://httpbin.org/get");
    printResponse(getResponse, "GET");

    // Пример 2: POST запрос с JSON
    std::cout << "Making POST request to httpbin.org..." << std::endl;
    std::string jsonPayload = R"({"name":"test","value":123})";
    HttpHeaders postHeaders;
    postHeaders.addHeader("Accept", "application/json");

    HttpResponse postResponse =
        client.Post("http://httpbin.org/post", jsonPayload, "application/json", postHeaders);
    printResponse(postResponse, "POST");

    // Пример 3: PUT запрос
    std::cout << "Making PUT request to httpbin.org..." << std::endl;
    std::string putPayload = "updated data";
    HttpResponse putResponse = client.Put("http://httpbin.org/put", putPayload, "text/plain");
    printResponse(putResponse, "PUT");

    // Пример 4: DELETE запрос
    std::cout << "Making DELETE request to httpbin.org..." << std::endl;
    HttpResponse deleteResponse = client.Delete("http://httpbin.org/delete");
    printResponse(deleteResponse, "DELETE");

    // Пример 5: GET запрос с пользовательскими заголовками
    std::cout << "Making GET request with custom headers..." << std::endl;
    HttpHeaders customHeaders;
    customHeaders.addHeader("X-Custom-Header", "test-value");
    customHeaders.addHeader("Accept-Language", "en-US,en;q=0.9");

    HttpResponse customGetResponse = client.Get("http://httpbin.org/headers", customHeaders);
    printResponse(customGetResponse, "GET with Custom Headers");

    // Пример 6: Асинхронный GET запрос
    std::cout << "Making async GET request..." << std::endl;
    auto asyncThread = client.getAsync("http://httpbin.org/delay/2", HttpHeaders(), asyncCallback);

    if (asyncThread) {
        std::cout << "Async request started, waiting for completion..." << std::endl;
        asyncThread->join();
        std::cout << "Async request completed!" << std::endl;
    }

    // Пример 7: Асинхронный POST запрос
    std::cout << "Making async POST request..." << std::endl;
    auto asyncPostThread = client.postAsync(
        "http://httpbin.org/post", "async test data", "text/plain", HttpHeaders(),
        [](const HttpResponse& response) {
            std::cout << "Async POST callback - Status: " << response.httpCode << std::endl;
        });

    if (asyncPostThread) {
        std::cout << "Async POST request started, waiting for completion..." << std::endl;
        asyncPostThread->join();
        std::cout << "Async POST request completed!" << std::endl;
    }

    // Пример 8: Обработка ошибок
    std::cout << "Testing error handling with invalid URL..." << std::endl;
    HttpResponse errorResponse = client.Get("http://invalid-domain-that-does-not-exist.com");
    if (errorResponse.httpCode == -1) {
        std::cout << "Successfully handled connection error!" << std::endl;
    }

    // Пример 9: Работа с URL с параметрами
    std::cout << "Making GET request with URL parameters..." << std::endl;
    HttpResponse paramResponse = client.Get("http://httpbin.org/get?param1=value1&param2=value2");
    printResponse(paramResponse, "GET with Parameters");

    // Пример 10: POST с form data
    std::cout << "Making POST request with form data..." << std::endl;
    std::string formData = "field1=value1&field2=value2&field3=value3";
    HttpResponse formResponse =
        client.Post("http://httpbin.org/post", formData, "application/x-www-form-urlencoded");
    printResponse(formResponse, "POST with Form Data");

    std::cout << "\nAll examples completed successfully!" << std::endl;
    return 0;
}
