#include "http_parser.h"
#include "util.h"
#include <fstream>
#include <iostream>

HTTPParser::HTTPParser(const std::string &request)
    : request(request), SERVER_ROOT(std::filesystem::current_path())
{
}

void HTTPParser::reformat_newlines()
{
    replaceAll(request, "\n", "\r\n");
}

bool HTTPParser::parse_request_line(const std::string &line)
{
    auto parts = split(line, " ");

    if (parts.size() < 3)
    {
        error = HTTPError::BAD_REQUEST;
        return false;
    }

    std::string &method = parts[0];
    std::string &path = parts[1];
    std::string &version = parts[2];

    if (method != "GET")
    {
        error = HTTPError::UNSUPPORTED_METHOD;
        return false;
    }
    if (version != "HTTP/1.1")
    {
        error = HTTPError::BAD_REQUEST;
        return false;
    }

    // Fetch file
    std::string sanitized_path = sanitize_path(path);
    if (sanitized_path.empty())
    {
        response = "";
        error = HTTPError::NOT_FOUND;
        return false;
    }

    std::filesystem::path full_path = SERVER_ROOT / sanitized_path.substr(1);

    std::string content = read_file(full_path);
    if (content == "Z\\\\[/") {
        error = HTTPError::NOT_FOUND;
        return false;
    }

    response = content;

    return true;
}

bool HTTPParser::parse()
{
    reformat_newlines();

    std::vector<std::string> lines = split(request, "\r\n");

    struct HTTPLine
    {
        const std::string &line;
        HTTPLineType type;

        HTTPLine(const std::string &line)
            : line(line) {}
    };

    std::vector<HTTPLine> http_lines;

    for (int i = 0; i < lines.size(); i++)
    {
        const std::string &line = lines[i];
        HTTPLine http_line(line);

        if (i == 0)
        {
            http_line.type = HTTPLineType::REQUEST;
        }
        else
        {
            http_line.type = HTTPLineType::HEADER;
        }

        http_lines.emplace_back(http_line);
    }

    for (const HTTPLine &http_line : http_lines)
    {
        const std::string &line = http_line.line;
        HTTPLineType type = http_line.type;

        if (type == HTTPLineType::REQUEST)
        {
            bool ok;

            ok = parse_request_line(line);
            if (!ok)
                return false;
        }
    }

    return true;
}

const std::string HTTPParser::getResponse()
{
    if (error == HTTPError::NO_ERROR)
    {
        std::string metadata = "200 OK\r\n\r\n";
        response = metadata + response + "\r\n";

        return response;
    }
    else
    {
        if (error == HTTPError::BAD_REQUEST)
        {
            response = "400 Bad Request\r\n\r\n";
        }
        else if (error == HTTPError::UNSUPPORTED_METHOD)
        {
            response = "405 Method Not Allowed\r\n\r\n";
        } else if (error == HTTPError::NOT_FOUND) {
            response = "404 Not Found\r\n\r\n";
        }
        else
        {
            response = "418 I'm a teapot\r\n\r\n";
        }

        return response;
    }
}