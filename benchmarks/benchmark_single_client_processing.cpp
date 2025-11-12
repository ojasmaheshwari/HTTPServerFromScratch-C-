//
// Created by epicman on 31/10/25.
//

#include <benchmark/benchmark.h>
#include "../server.h"
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include "../vendor/logging/include/Logging.h"
#include <iostream>
#include "../http_parser.h"
#include "../util.h"

int PORT = 8080;
const char* SERVER_ADDRESS = "127.0.0.1";
int THREAD_POOL_SIZE = 20;

// Fake function to simulate read/write to sockets
std::string fake_http_request() {
    return "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\n";
}

void process_request(const int client_socket_fd, const std::string &client_ip_addr, Logging &logger, const int client_port, const std::string &request) {
    HTTPParser parser(request);
    if (!parser.parse()) {
        std::cout << "[!] FAILED TO PARSE REQUEST\n";
    }

    std::string response = parser.getResponse();
    const char *response_buffer = response.c_str();
}

// This function benchmarks handle_client() directly, using a pipe() to simulate a client socket
static void BM_FromRequestToResponse(benchmark::State& state) {
    int sv[2]; // socketpair
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        exit(1);
    }

    sockaddr_in dummy_addr{};
    dummy_addr.sin_family = AF_INET;
    dummy_addr.sin_port = htons(8080);
    dummy_addr.sin_addr.s_addr = INADDR_ANY;

    // Write a fake HTTP request into one side of the socket
    std::string req = fake_http_request();

    Logging logger;
    logger.setClassName("Benchmark");

    const std::string client_ip = "127.0.0.1";
    const int client_port = 8080;

    for (auto _ : state) {

        // Benchmark the processing of a request
        // Add arguments here
        process_request(sv[1], client_ip, logger, client_port, "GET /server HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n");
    }
}
BENCHMARK(BM_FromRequestToResponse);
BENCHMARK_MAIN();
