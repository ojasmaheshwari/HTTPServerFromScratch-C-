#include "http_parser.h"
#include "thread_pool.h"
#include "util.h"
#include "vendor/logging/include/Logging.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <signal.h>
#include <string>
#include <thread>
#include <unistd.h>

int PORT = 8080;
const char* SERVER_ADDRESS = "127.0.0.1";
int THREAD_POOL_SIZE = 20;

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

int main(int argc, char *argv[]) {
  // Check if user provides port, ip address, thread pool size
  if (argc == 4) {
    PORT = std::stoi(argv[1]);
    SERVER_ADDRESS = argv[2];
    THREAD_POOL_SIZE = std::stoi(argv[3]);
  }

  // If the browser closes the connection then we write to a broken pipe
  // In that case SIGPIPE will be thrown
  // We ignore that and just log that the server closed connection and then
  // accept new connections
  signal(SIGPIPE, SIG_IGN);

  Logging logger;
  logger.setClassName("main");

  ThreadPool pool(THREAD_POOL_SIZE);

  // Integer return value used for validation of errors
  int ret_val;

  // Create socket. It returns a file descriptor which is a normal integer
  // pointing to an open file in OS i.e our open socket
  int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_fd == -1) {
    std::cerr << "Error creating socket object\n";
    exit(EXIT_FAILURE);
  }

  // Set socket options
  // We allow reusing addresses to avoid 'Address already in use' errors
  int op_val = 1;
  ret_val =
      setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &op_val, sizeof(op_val));
  if (ret_val == -1) {
    perror("setsockopt() failed");
    exit(EXIT_FAILURE);
  }

  // Create address struct and populate it
  // It's called sockaddr_in because it's an address struct for IPv4. For IPv6,
  // you would have to use 'sockaddr_in6' It's also important to note that
  // whenever a 'sin' or 'in' prefix/suffix is used, it likely evaluates to
  // 'internet sockets IPv4'
  sockaddr_in address;
  address.sin_family = AF_INET;

  // If we simply write address.sin_port = PORT; then we are storing the port in
  // host byte order (defaults to little endian) But the socket libraries use
  // network byte order which is big endian So we use htons() to convert our
  // integer from host byte order to network byte order
  address.sin_port = htons(PORT);

  // Now we need to store an IP address inside address.sin_addr
  // For that we make use of 'inet_pton' which converts a given IP address in
  // string form to binary We need to do this as address.sin_addr resolves to
  // 32-bit unsigned int
  ret_val = inet_pton(AF_INET, SERVER_ADDRESS, &address.sin_addr);
  if (ret_val == 0) {
    std::cerr << "The IP address provided is not a valid IPv4 address\n";
    exit(EXIT_FAILURE);
  } else if (ret_val == -1) {
    std::cerr << "Invalid address family provided to inet_pton\n";
    exit(EXIT_FAILURE);
  }

  // Bind the socket
  // Note that we also need to explicitly pass the length of address struct here
  // as the second argument is a generic pointer
  ret_val = bind(socket_fd, (sockaddr *)(&address), sizeof(address));
  if (ret_val == -1) {
    perror("Binding the socket failed");
    exit(EXIT_FAILURE);
  }

  // Listen for connections
  ret_val = listen(socket_fd, 20);
  ;
  if (ret_val == -1) {
    std::cerr << "Failed to call listen() on sockets\n";
    exit(EXIT_FAILURE);
  }

  logger.log("HTTP Server started on http://" + std::string(SERVER_ADDRESS) +
             ":" + std::to_string(PORT));
  logger.log("Serving files from 'res' directory");
  logger.log("Press Ctrl+C to stop the server");

  while (true) {
    // Since accept returns a socket file descriptor attached to the client
    // We also need to provide it with pointers to new sockaddr and socklen_t
    // structs to have information about the client
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int client_socket_fd = accept(socket_fd, (sockaddr *)(&client_address),
                                  (socklen_t *)&client_address_len);
    if (client_socket_fd == -1) {
      perror("accept() call failed. Connection with client failed\n");
      exit(EXIT_FAILURE);
    }

    // std::thread(handle_client, client_address, client_socket_fd).detach();
    pool.enqueue([client_address, client_socket_fd]() {
      handle_client(client_address, client_socket_fd);
    });
  }

  close(socket_fd);
}