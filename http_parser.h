#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <filesystem>

enum HTTPLineType {
    REQUEST = 0,
    HEADER,
    BODY
};

enum HTTPError {
    BAD_REQUEST = 0,
    UNSUPPORTED_METHOD,
    INVALID_HTTP_VERSION,
    NO_ERROR,
    NOT_FOUND,
    FORBIDDEN
};

class HTTPParser
{
private:
    std::string request;
    HTTPError error = NO_ERROR;
    std::filesystem::path SERVER_ROOT;
    std::string response;

    // Request information variables
    std::string http_method;
    std::string http_route;
    std::string http_version;
    std::unordered_map<std::string, std::string> http_headers;
    std::string http_body;

public:
    HTTPParser(const std::string &request);

    // Replaces all newlines with \r\n
    void reformat_newlines();

    // Parsing functions
    bool parse_request_line(const std::string &line);
    bool parse();
    bool validate_fields();

    const std::string getResponse();
};