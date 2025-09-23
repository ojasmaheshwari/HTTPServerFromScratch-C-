#include "util.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

const std::string receive_line(int socket_fd, int MAX_SIZE)
{
    char buffer[MAX_SIZE + 1];
    int bytes_read = read(socket_fd, &buffer, MAX_SIZE);

    if (bytes_read == -1)
    {
        perror("util.h - read() failed");
        exit(EXIT_FAILURE);
    }

    buffer[bytes_read] = '\0';

    return buffer;
}

const std::string receive_http_req(int socket_fd, int MAX_SIZE)
{
    char buffer[MAX_SIZE + 1];
    int bytes_read = read(socket_fd, &buffer, MAX_SIZE);

    if (bytes_read == -1)
    {
        perror("util.h - receive_http_req() failed");
        exit(EXIT_FAILURE);
    }

    buffer[bytes_read] = '\0';

    return buffer;
}

void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
        return;

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

std::vector<std::string> split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length(); // move past the delimiter
        end = str.find(delimiter, start);
    }

    result.push_back(str.substr(start)); // add the last part
    return result;
}

const std::string sanitize_path(const std::string &path)
{
    std::filesystem::path requested(path);

    requested = requested.lexically_normal();

    if (requested.string().find("..") != std::string::npos)
    {
        return "";
    }

    return requested.string();
}

const std::string read_file(const std::string &path)
{
    std::ifstream file(path);

    if (!file.is_open()) {
        perror("read_file() failed");
        return "Z\\\\[/";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string content = buffer.str();

    return content;
}