#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "http_parser.h"
#include "util.h"

int main()
{
    // Integer return value used for validation of errors
    int ret_val;

    // Create socket. It returns a file descriptor which is a normal integer pointing to an open file in OS i.e our open socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1)
    {
        std::cerr << "Error creating socket object\n";
        exit(EXIT_FAILURE);
    }

    // Set socket options
    // We allow reusing addresses to avoid 'Address already in use' errors
    int op_val = 1;
    ret_val = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &op_val, sizeof(op_val));
    if (ret_val == -1)
    {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    // Create address struct and populate it
    // It's called sockaddr_in because it's an address struct for IPv4. For IPv6, you would have to use 'sockaddr_in6'
    // It's also important to note that whenever a 'sin' or 'in' prefix/suffix is used, it likely evaluates to 'internet sockets IPv4'
    sockaddr_in address;
    address.sin_family = AF_INET;

    // If we simply write address.sin_port = 8080; then we are storing the port in host byte order (defaults to little endian)
    // But the socket libraries use network byte order which is big endian
    // So we use htons() to convert our integer from host byte order to network byte order
    address.sin_port = htons(8080);

    // Now we need to store an IP address inside address.sin_addr
    // For that we make use of 'inet_pton' which converts a given IP address in string form to binary
    // We need to do this as address.sin_addr resolves to 32-bit unsigned int
    ret_val = inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    if (ret_val == 0)
    {
        std::cerr << "The IP address provided is not a valid IPv4 address\n";
        exit(EXIT_FAILURE);
    }
    else if (ret_val == -1)
    {
        std::cerr << "Invalid address family provided to inet_pton\n";
        exit(EXIT_FAILURE);
    }

    // Bind the socket
    // Note that we also need to explicitly pass the length of address struct here as the second argument is a generic pointer
    ret_val = bind(socket_fd, (sockaddr *)(&address), sizeof(address));
    if (ret_val == -1)
    {
        perror("Binding the socket failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    ret_val = listen(socket_fd, 5);
    ;
    if (ret_val == -1)
    {
        std::cerr << "Failed to call listen() on sockets\n";
        exit(EXIT_FAILURE);
    }

    // Since accept returns a socket file descriptor attached to the client
    // We also need to provide it with pointers to new sockaddr and socklen_t structs to have information about the client
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int client_socket_fd = accept(socket_fd, (sockaddr *)(&client_address), (socklen_t *)&client_address_len);
    if (client_socket_fd == -1)
    {
        std::cerr << "accept() call failed. Connection with client failed\n";
        exit(EXIT_FAILURE);
    }

    // Print the client's IP address by converting the address from binary format to a char*
    char client_ip_addr[100];
    const char *ret_ntop = inet_ntop(AF_INET, &client_address.sin_addr, client_ip_addr, 100);
    if (ret_ntop == NULL)
    {
        perror("inet_ntop() failed, unable to get client address");
        exit(EXIT_FAILURE);
    }

    std::cout << "Connection from: " << client_ip_addr << '\n';

    // Read incoming data
    // Note: read() does not null terminate the array
    // We have to do it ourselves, so always read 1 less byte than the size of your buffer
    while (true)
    {
        std::string request = receive_http_req(client_socket_fd);

        std::cout << "--- REQUEST START ---\n";
        std::cout << request;
        std::cout << "--- REQUEST END ---\n";

        HTTPParser parser(request);
        if (!parser.parse()) {
            std::cerr << "--- [!] FAILED TO PARSE REQUEST ---\n";
        }

        std::string response = parser.getResponse();
        const char* response_buffer = response.c_str();

        write(client_socket_fd, response_buffer, response.size());
    }

    close(client_socket_fd);
    close(socket_fd);
}