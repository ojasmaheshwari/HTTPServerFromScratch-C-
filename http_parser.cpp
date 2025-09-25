#include "http_parser.h"
#include "http_response_builder.h"
#include "util.h"
#include "vendor/logging/include/Logging.h"
#include "vendor/nlohmann/json.hpp"
#include <chrono>
#include <filesystem>
#include <set>
#include <string>

using json = nlohmann::json;

HTTPParser::HTTPParser(const std::string &request)
    : request(request), SERVER_ROOT(std::filesystem::current_path() / "res") {}

bool HTTPParser::parse() {
  Logging logger;
  logger.setClassName("HTTPParser::parse()");

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
    logger.warn("Malformed HTTP Request. An split on \\r\\n was attempted to "
                "separate request metadata from body and the split wasn't "
                "successfull. The error occured on the below request");
    logger.warn(request);
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
    logger.warn("Malformed HTTP Request. Tried to split request_metadata on "
                "\\r\\n but no lines were returned.");
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
    logger.warn("Malformed HTTP Request. Tried to split the request line on "
                "space but failed.");
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
      logger.warn("Malformed Header. Tried to split header on ': ' but failed. "
                  "Check the header string below.");
      logger.warn(header_str);
      return false;
    }

    http_headers[data[0]] = data[1];
  }

  // STEP 5
  bool is_valid_request = validate_fields();
  if (!is_valid_request) {
    return false;
  }
  bool is_processing_successfull = process_request();

  return is_processing_successfull;
}

// Validate data
// Following are the things that need to be validated:-
//      i) Request method - Must be GET or POST
//      ii) HTTP Version - Must be HTTP/1.1 or HTTP/1.0
//      iii) Host header must be present with value = localhost:<PORT> or
//      <SERVER_ADDRESS>:<PORT>
bool HTTPParser::validate_fields() {
  Logging logger;
  logger.setClassName("HTTPParser::validate_fields()");

  std::set<std::string> allowed_methods = {"GET", "POST"};
  std::set<std::string> allowed_http_versions = {"HTTP/1.1", "HTTP/1.0"};

  if (allowed_methods.count(http_method) == 0) {
    status = HTTPStatus::UNSUPPORTED_METHOD;
    logger.warn("Unknown HTTP method. The below given method was provided");
    logger.warn(http_method);
    return false;
  }
  if (allowed_http_versions.count(http_version) == 0) {
    status = HTTPStatus::BAD_REQUEST;
    logger.warn(
        "Unknown HTTP version. The below given HTTP version was provided");
    logger.warn(http_version);
    return false;
  }
  if (http_headers.count("Host") == 0) {
    status = HTTPStatus::BAD_REQUEST;
    logger.warn("Host header not found.");
    return false;
  }
  const auto host = http_headers["Host"];
  std::string correct_host_value =
      std::string(SERVER_ADDRESS) + ":" + std::to_string(PORT);
  if (host != correct_host_value) {
    status = HTTPStatus::FORBIDDEN;
    logger.warn("Host mismatch. The below given host was provided");
    logger.warn("Got host: " + host + " Expected host: " + correct_host_value);
    return false;
  }

  return true;
}

bool HTTPParser::process_GET_request() {
  Logging logger;
  logger.setClassName("HTTPParser::process_GET_request");

  // GET requests are used for fetching of files
  // First of all we need to sanitize the route we recieved in order to protect
  // against path traversals
  auto sanitized_route = sanitize_path(http_route);
  if (!sanitized_route) {
    status = HTTPStatus::FORBIDDEN;
    logger.warn("Path traversal attempt blocked. Route tried to escape the "
                "SERVER ROOT");
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
  }

  auto file = read_file(fullpath);
  if (!file) {
    status = HTTPStatus::NOT_FOUND;
    logger.warn("Requested file not found - " + fullpath.string());
    return false;
  }

  std::string file_content = file.value();

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
  } else if (extension == ".json") {
    content_type = HTTPContentType::JSON;
  } else if (extension == ".js") {
    content_type = HTTPContentType::JS;
  } else if (extension == ".css") {
    content_type = HTTPContentType::CSS;
  } else {
    content_type = HTTPContentType::OCTET_STREAM;
  }

  // If user requested an actual file then set http_requested_filename
  if (fullpath.has_filename()) {
    http_requested_filename = fullpath.filename();
  }
  response_body = file_content;

  return true;
}

bool HTTPParser::process_POST_request() {
  Logging logger;
  logger.setClassName("HTTPParser::process_POST_request");

  // We only process JSON data in POST requests
  // First of all, check whether the Content-Type of the incoming request is
  // application/json

  if (http_headers.count("Content-Type") == 0) {
    status = HTTPStatus::BAD_REQUEST;
    logger.warn("POST request does not contain the Content-Type header");
    return false;
  }

  if (http_headers["Content-Type"] != "application/json") {
    status = HTTPStatus::UNSUPPORTED_MEDIA_TYPE;
    logger.warn(
        "Content-Type header of incoming POST request is not application/json");
    return false;
  }

  auto parsedJson = json::parse(http_body, nullptr, false);
  if (parsedJson.is_discarded()) {
    status = HTTPStatus::BAD_REQUEST;
    logger.warn("POST request contains invalid JSON");
    return false;
  }

  // Now that the JSON is valid
  // We can simply write the JSON content to res/uploads
  auto current_ts =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto uid = generate_random_id(10);
  std::string filename =
      "upload_" + std::to_string(current_ts) + "_" + uid + ".json";
  std::filesystem::path path_to_write = SERVER_ROOT / "uploads" / filename;

  bool written = write_file(http_body, path_to_write);
  if (!written) {
    status = HTTPStatus::INTERNAL_SERVER_ERROR;
    logger.warn("Failed to write to file in POST request");
    return false;
  }

  status = HTTPStatus::CREATED;

  std::string json_response =
      std::string("{ \"status\" : \"success\", \"message\" : \"File created "
                  "successfully\", \"filepath\" : \"uploads/") +
      filename + "\" }";
  response_body = json_response;

  return true;
}

bool HTTPParser::process_request() {
  Logging logger;
  logger.setClassName("HTTPParser::process_request");

  if (http_method == "GET") {
    return process_GET_request();
  } else if (http_method == "POST") {
    return process_POST_request();
  } else {
    status = HTTPStatus::BAD_REQUEST;
    logger.warn("Unknown request method used - " + http_method);
    return false;
  }
}

const std::string HTTPParser::getResponse() {
  HTTPResponseBuilder builder(http_version, status, response_body, content_type,
                              http_headers, http_requested_filename);
  auto response = builder.build();
  return response;
}