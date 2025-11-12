//
// Created by epicman on 31/10/25.
//

#pragma once

#include <netinet/in.h>


void handle_client(sockaddr_in client_address, int client_socket_fd);