#pragma once

#include <string>
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
    NOT_FOUND
};

class HTTPParser
{
private:
    std::string request;
    HTTPError error = NO_ERROR;
    std::filesystem::path SERVER_ROOT;
    std::string response;

public:
    HTTPParser(const std::string &request);

    // Replaces all newlines with \r\n
    void reformat_newlines();

    // Parsing functions
    bool parse_request_line(const std::string &line);
    bool parse();

    const std::string getResponse();
};