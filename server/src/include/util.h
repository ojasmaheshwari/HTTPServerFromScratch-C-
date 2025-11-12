#pragma once

#include <optional>
#include <string>
#include <vector>

extern int PORT;
extern const char *SERVER_ADDRESS;
extern int THREAD_POOL_SIZE;

const std::string receive_line(int socket_fd, int MAX_SIZE = 1024);
const std::string receive_http_req(int socket_fd, int MAX_SIZE = 2048);
void replaceAll(std::string &str, const std::string &from,
                const std::string &to);
std::vector<std::string> split(const std::string &str,
                               const std::string &delimiter);
const std::optional<std::string> sanitize_path(const std::string &path);
const std::optional<std::string> read_file(const std::string &path);
bool write_file(const std::string &content, const std::string &path);
std::string generate_random_id(size_t length);
std::string get_rfc7231_date();
