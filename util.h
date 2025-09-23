#pragma once

#include <string>
#include <unistd.h>
#include <vector>

const std::string receive_line(int socket_fd, int MAX_SIZE = 1024);
const std::string receive_http_req(int socket_fd, int MAX_SIZE = 2048);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
std::vector<std::string> split(const std::string& str, const std::string& delimiter);
const std::string sanitize_path(const std::string &path);
const std::string read_file(const std::string &path);