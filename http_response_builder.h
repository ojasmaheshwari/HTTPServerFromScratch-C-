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
public:
    HTTPResponseBuilder(const std::string &version, HTTPStatus status, const std::string& response_body, HTTPContentType content_type);
    std::string build();
};