#include "http_parser.h"
#include "http_response_builder.h"
#include "util.h"
#include <filesystem>
#include <iostream>
#include <set>

HTTPParser::HTTPParser(const std::string &request)
    : request(request), SERVER_ROOT(std::filesystem::current_path() / "res") {}

// bool HTTPParser::parse_request_line(const std::string &line) {
//   auto parts = split(line, " ");

//   if (parts.size() < 3) {
//     error = HTTPError::BAD_REQUEST;
//     return false;
//   }

//   std::string &method = parts[0];
//   std::string &path = parts[1];
//   std::string &version = parts[2];

//   if (method != "GET") {
//     error = HTTPError::UNSUPPORTED_METHOD;
//     return false;
//   }
//   if (version != "HTTP/1.1") {
//     error = HTTPError::BAD_REQUEST;
//     return false;
//   }

//   // Fetch file
//   std::string sanitized_path = sanitize_path(path);
//   if (sanitized_path.empty()) {
//     response = "";
//     error = HTTPError::NOT_FOUND;
//     return false;
//   }

//   std::filesystem::path full_path = SERVER_ROOT / sanitized_path.substr(1);

//   std::string content = read_file(full_path);
//   if (content == "Z\\\\[/") {
//     error = HTTPError::NOT_FOUND;
//     return false;
//   }

//   response = content;

//   return true;
// }

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
    status = HTTPStatus::BAD_REQUEST;
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
    status = HTTPStatus::BAD_REQUEST;
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
    status = HTTPStatus::BAD_REQUEST;
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
      status = HTTPStatus::BAD_REQUEST;
      return false;
    }

    http_headers[data[0]] = data[1];
  }

  // STEP 5
  bool is_valid_request = validate_fields();
  bool is_processing_successfull = process_request();

  return is_valid_request && is_processing_successfull;
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
    status = HTTPStatus::UNSUPPORTED_METHOD;
    return false;
  }
  if (allowed_http_versions.count(http_version) == 0) {
    status = HTTPStatus::BAD_REQUEST;
    return false;
  }
  if (http_headers.count("Host") == 0) {
    status = HTTPStatus::BAD_REQUEST;
    return false;
  }
  const auto host = http_headers["Host"];
  if (host != "localhost:9173") {
    status = HTTPStatus::FORBIDDEN;
    return false;
  }

  return true;
}

bool HTTPParser::process_GET_request() {
  // GET requests are used for fetching of files
  // First of all we need to sanitize the route we recieved in order to protect
  // against path traversals

  auto sanitized_route = sanitize_path(http_route);
  if (!sanitized_route) {
    status = HTTPStatus::FORBIDDEN;
    return false;
  }

  std::string route = sanitized_route.value();

  // Check if the route is '/'
  // Because in that case we need to check the presence of an index.html file
  // and serve it
  std::filesystem::path fullpath;
  if (route == "/") {
    fullpath = SERVER_ROOT / "index.html";
    content_type = HTTPContentType::HTML;
  } else {
    fullpath = SERVER_ROOT / route.substr(1);
    
    // Add hints based on file extension
    // It would be much better to do it by scanning the file content
    // But doing this for simplicity and PoC
    auto extension = fullpath.extension();
    if (extension == ".html") {
      content_type = HTTPContentType::HTML;
    } else if (extension == ".png") {
      content_type = HTTPContentType::PNG;
    } else if (extension == ".jpg") {
      content_type = HTTPContentType::JPG;
    } else if (extension == ".jpeg") {
      content_type = HTTPContentType::JPEG;
    } else if (extension == ".gif") {
      content_type = HTTPContentType::GIF;
    }
  }

  auto file = read_file(fullpath);
  if (!file) {
    status = HTTPStatus::NOT_FOUND;
    return false;
  }

  std::string file_content = file.value();

  response_body = file_content;

  return true;
}

bool HTTPParser::process_POST_request() { return true; }

bool HTTPParser::process_request() {
  if (http_method == "GET") {
    return process_GET_request();
  } else if (http_method == "POST") {
    return process_POST_request();
  } else {
    status = HTTPStatus::BAD_REQUEST;
    return false;
  }
}

const std::string HTTPParser::get_error_response() {
  if (status == HTTPStatus::BAD_REQUEST) {
    response = "HTTP/1.1 400 Bad Request\r\n\r\n";
  } else if (status == HTTPStatus::UNSUPPORTED_METHOD) {
    response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
  } else if (status == HTTPStatus::NOT_FOUND) {
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
  } else if (status == HTTPStatus::FORBIDDEN) {
    response = "HTTP/1.1 403 Forbidden\r\n\r\n";
  } else {
    response = "HTTP/1.1 418 I'm a teapot\r\n\r\n";
  }

  return response;
}

const std::string HTTPParser::getResponse() {
  HTTPResponseBuilder builder(http_version, status, response_body, content_type);
  auto response = builder.build();
  return response;
}