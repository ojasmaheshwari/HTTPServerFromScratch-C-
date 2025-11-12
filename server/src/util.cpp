#include <util.h>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <random>
#include <unistd.h>

const std::string receive_line(int socket_fd, int MAX_SIZE) {
  char buffer[MAX_SIZE + 1];
  int bytes_read = read(socket_fd, &buffer, MAX_SIZE);

  if (bytes_read == -1) {
    perror("util.h - read() failed");
    exit(EXIT_FAILURE);
  }

  buffer[bytes_read] = '\0';

  return buffer;
}

const std::string receive_http_req(int socket_fd, int MAX_SIZE) {
  char buffer[MAX_SIZE + 1];
  int bytes_read = read(socket_fd, &buffer, MAX_SIZE);

  if (bytes_read == -1) {
    perror("util.h - receive_http_req() failed");
    exit(EXIT_FAILURE);
  }

  buffer[bytes_read] = '\0';

  return buffer;
}

void replaceAll(std::string &str, const std::string &from,
                const std::string &to) {
  if (from.empty())
    return;

  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

std::vector<std::string> split(const std::string &str,
                               const std::string &delimiter) {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = str.find(delimiter);

  while (end != std::string::npos) {
    result.push_back(str.substr(start, end - start));
    start = end + delimiter.length(); // move past the delimiter
    end = str.find(delimiter, start);
  }

  result.push_back(str.substr(start)); // add the last part
  return result;
}

const std::optional<std::string> sanitize_path(const std::string &path) {
  std::filesystem::path requested(path);

  requested = requested.lexically_normal();

  if (requested.string().find("..") != std::string::npos) {
    return std::nullopt;
  }

  return requested.string();
}

const std::optional<std::string> read_file(const std::string &path) {
  std::ifstream file(path);

  if (!file.is_open()) {
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  std::string content = buffer.str();

  return content;
}

bool write_file(const std::string& content, const std::string &path) {
    std::ofstream file(path);

    if (!file.is_open()) {
        return false;
    }

    file << content;

    file.close();

    return true;
}

std::string generate_random_id(size_t length) {
    static const std::string chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, chars.size() - 1);

    std::string id;
    id.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        id += chars[dist(rng)];
    }

    return id;
}

std::string get_rfc7231_date() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::gmtime(&t); // GMT time
    char buf[30];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return std::string(buf);
}