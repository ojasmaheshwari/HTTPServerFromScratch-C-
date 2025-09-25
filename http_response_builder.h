#pragma once

#include "http_parser.h"
#include <map>
#include <string>

class HTTPResponseBuilder {
private:
    std::string version;
    HTTPStatus status;
    std::string response_body;
    HTTPContentType content_type;
    std::map<HTTPStatus, std::string> httpcode_string_map;
    std::map<HTTPContentType, std::string> contenttype_string_map;

    // Default body content for error status codes
    std::string forbidden_body = "<!DOCTYPE html><html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1><p>You don't have permission to access this resource.</p></body></html>";
    std::string not_found_body = "<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><p>The requested resource could not be found on this server.</p></body></html>";
    std::string bad_request_body = "<!DOCTYPE html><html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1><p>Your browser sent a request that this server could not understand.</p></body></html>";
    std::string method_not_allowed_body = "<!DOCTYPE html><html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1><p>The request method is not supported for this resource.</p></body></html>";
public:
    HTTPResponseBuilder(const std::string &version, HTTPStatus status, const std::string& response_body, HTTPContentType content_type);
    std::string build();
};