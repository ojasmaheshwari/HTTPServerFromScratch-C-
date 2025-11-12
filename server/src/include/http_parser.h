#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <filesystem>

enum HTTPLineType {
    REQUEST = 0,
    HEADER,
    BODY
};

enum HTTPStatus {
    BAD_REQUEST = 0,
    UNSUPPORTED_METHOD,
    OK,
    NOT_FOUND,
    FORBIDDEN,
    UNSUPPORTED_MEDIA_TYPE,
    INTERNAL_SERVER_ERROR,
    CREATED
};

enum HTTPContentType {
    HTML = 0,
    PNG,
    JPG,
    JPEG,
    GIF,
    ICO,
    TEXT,
    JSON,
    OCTET_STREAM,
    JS,
    CSS
};

class HTTPParser
{
private:
    std::string request;
    HTTPStatus status = HTTPStatus::OK;
    std::filesystem::path SERVER_ROOT;
    std::string response;

    // Request information variables
    std::string http_method;
    std::string http_route;
    std::string http_version;
    std::unordered_map<std::string, std::string> http_headers;
    std::string http_body;
    std::optional<std::string> http_requested_filename;

    // Response information variables
    std::string response_body;

    // Content type for response
    HTTPContentType content_type = HTTPContentType::TEXT;
    

public:
    HTTPParser(const std::string &request);

    // Replaces all newlines with \r\n
    void reformat_newlines();

    // Parsing functions
    bool parse();
    bool validate_fields();

    // Function to process the request
    bool process_request();
    bool process_GET_request();
    bool process_POST_request();

    // Response functions
    const std::string getResponse();
};