#include "http_parser.h"
#include "util.h"
#include <iostream>
#include <set>
#include <utility>

HTTPParser::HTTPParser(const std::string &request)
    : request(request), SERVER_ROOT(std::filesystem::current_path()) {}

bool HTTPParser::parse_request_line(const std::string &line) {
  auto parts = split(line, " ");

  if (parts.size() < 3) {
    error = HTTPError::BAD_REQUEST;
    return false;
  }

  std::string &method = parts[0];
  std::string &path = parts[1];
  std::string &version = parts[2];

  if (method != "GET") {
    error = HTTPError::UNSUPPORTED_METHOD;
    return false;
  }
  if (version != "HTTP/1.1") {
    error = HTTPError::BAD_REQUEST;
    return false;
  }

  // Fetch file
  std::string sanitized_path = sanitize_path(path);
  if (sanitized_path.empty()) {
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

bool HTTPParser::parse() {
  // Plan: Parse the request string into an object
  //         The object should contain the following:-
  //              i) Request line - String
  //              ii) Headers - Array of string
  //              iii) Body - String (Optional datatype, present if request
  //              method is POST)

  // STEP 1
  // Split the body part from the request metadata
  auto parts = split(request, "\r\n\r\n");
  if (parts.size() != 2) {
    // Even in a GET request, this split should have been successfull
    // The request is malformed if this split is not successfull
    error = HTTPError::BAD_REQUEST;
    return false;
  }
  std::string request_metadata = parts[0];
  http_body = parts[1];

  // STEP 2
  // Parse the request_metadata to extract the request line and the headers
  // array
  std::string request_line;
  std::vector<std::string> headers_str;

  auto request_metadata_lines = split(request_metadata, "\r\n");
  if (request_metadata_lines.size() == 0) {
    error = HTTPError::BAD_REQUEST;
    return false;
  }
  request_line = request_metadata_lines[0];
  for (int i = 1; i < request_metadata_lines.size(); i++) {
    headers_str.push_back(request_metadata_lines[i]);
  }

  // STEP 3
  // Parse the request line to extract the following:-
  //      i) Request method
  //      ii) Route
  //      iii) HTTP Version
  auto request_line_data = split(request_line, " ");
  if (request_line_data.size() != 3) {
    error = HTTPError::BAD_REQUEST;
    return false;
  }
  http_method = request_line_data[0];
  http_route = request_line_data[1];
  http_version = request_line_data[2];

  // STEP 4
  // Parse the headers.
  // Currently they are strings, so split them and store them in key-value pairs
  for (const auto &header_str : headers_str) {
    auto data = split(header_str, ": ");
    if (data.size() != 2) {
      error = HTTPError::BAD_REQUEST;
      return false;
    }

    http_headers[data[0]] = data[1];
  }

  // STEP 5
  bool is_valid_request = validate_fields();

  return is_valid_request;
}

// Validate data
// Following are the things that need to be validated:-
//      i) Request method - Must be GET or POST
//      ii) HTTP Version - Must be HTTP/1.1 or HTTP/1.0
//      iii) Host header must be present with value = localhost:<PORT> or
//      <SERVER_ADDRESS>:<PORT>
bool HTTPParser::validate_fields() {
  std::set<std::string> allowed_methods = {"GET", "POST"};
  std::set<std::string> allowed_http_versions = {"HTTP/1.1", "HTTP/1.0"};

  if (allowed_methods.count(http_method) == 0) {
    error = HTTPError::UNSUPPORTED_METHOD;
    return false;
  }
  if (allowed_http_versions.count(http_version) == 0) {
    error = HTTPError::BAD_REQUEST;
    return false;
  }
  if (http_headers.count("Host") == 0) {
    error = HTTPError::BAD_REQUEST;
    return false;
  }
  const auto host = http_headers["Host"];
  if (host != "localhost:9173") {
    error = HTTPError::FORBIDDEN;
    return false;
  }

  return true;
}

const std::string HTTPParser::getResponse() {
  if (error == HTTPError::NO_ERROR) {
    std::string metadata = "HTTP/1.1 200 OK\r\n\r\n";
    response = metadata + response + "\r\n";

    return response;
  } else {
    if (error == HTTPError::BAD_REQUEST) {
      response = "HTTP/1.1 400 Bad Request\r\n\r\n";
    } else if (error == HTTPError::UNSUPPORTED_METHOD) {
      response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    } else if (error == HTTPError::NOT_FOUND) {
      response = "HTTP/1.1 404 Not Found\r\n\r\n";
    } else if (error == HTTPError::FORBIDDEN) {
      response = "HTTP/1.1 403 Forbidden\r\n\r\n";
    } else {
      response = "HTTP/1.1 418 I'm a teapot\r\n\r\n";
    }

    return response;
  }
}