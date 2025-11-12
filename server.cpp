//
// Created by epicman on 31/10/25.
//

#include "server.h"
#include "vendor/logging/include/Logging.h"
#include "http_parser.h"
#include <arpa/inet.h>
#include <iostream>
#include "util.h"

void handle_client(sockaddr_in client_address, int client_socket_fd) {
    Logging logger;
    logger.setClassName("handle_client");

    // Print the client's IP address by converting the address from binary
    // format to a char*
    char client_ip_addr[100];
    const char *ret_ntop =
        inet_ntop(AF_INET, &client_address.sin_addr, client_ip_addr, 100);
    if (ret_ntop == NULL) {
        perror("inet_ntop() failed, unable to get client address");
        exit(EXIT_FAILURE);
    }

    int client_port = ntohs(client_address.sin_port);

    logger.info(std::string("Connection from: ") + client_ip_addr + ":" +
                std::to_string(client_port));

    // Read incoming data
    // Note: read() does not null terminate the array
    // We have to do it ourselves, so always read 1 less byte than the size of
    // your buffer
    while (true) {
        std::string request = receive_http_req(client_socket_fd);

        HTTPParser parser(request);
        if (!parser.parse()) {
            std::cout << "[!] FAILED TO PARSE REQUEST\n";
        }

        std::string response = parser.getResponse();
        const char *response_buffer = response.c_str();

        int data_written =
            write(client_socket_fd, response_buffer, response.size());
        if (data_written == -1) {
            logger.log(std::string("Client ") + client_ip_addr + ":" +
                       std::to_string(client_port) + " closed connection");
            break;
        }
    }

    close(client_socket_fd);
}