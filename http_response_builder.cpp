#include "http_response_builder.h"
#include "http_parser.h"
#include "util.h"
#include <string>

HTTPResponseBuilder::HTTPResponseBuilder(
    const std::string &version, HTTPStatus status,
    const std::string &response_body, HTTPContentType content_type,
    std::unordered_map<std::string, std::string> &http_headers,
    std::optional<std::string> &http_requested_filename)
    : version(version), status(status), response_body(response_body),
      content_type(content_type), http_headers(http_headers),
      http_requested_filename(http_requested_filename) {
  httpcode_string_map[HTTPStatus::OK] = "200 OK";
  httpcode_string_map[HTTPStatus::NOT_FOUND] = "404 Not Found";
  httpcode_string_map[HTTPStatus::BAD_REQUEST] = "400 Bad Request";
  httpcode_string_map[HTTPStatus::FORBIDDEN] = "403 Forbidden";
  httpcode_string_map[HTTPStatus::UNSUPPORTED_METHOD] =
      "405 Method Not Allowed";
  httpcode_string_map[HTTPStatus::CREATED] = "201 Created";

  contenttype_string_map[HTTPContentType::HTML] = "text/html";
  contenttype_string_map[HTTPContentType::PNG] = "image/png";
  contenttype_string_map[HTTPContentType::JPG] = "image/jpg";
  contenttype_string_map[HTTPContentType::JPEG] = "image/jpeg";
  contenttype_string_map[HTTPContentType::GIF] = "image/gif";
  contenttype_string_map[HTTPContentType::ICO] = "image/x-icon";
  contenttype_string_map[HTTPContentType::TEXT] = "text/plain";
  contenttype_string_map[HTTPContentType::JSON] = "application/json";
  contenttype_string_map[HTTPContentType::OCTET_STREAM] =
      "application/octet-stream";
      contenttype_string_map[HTTPContentType::JS] = "application/javascript";
      contenttype_string_map[HTTPContentType::CSS] = "text/css";
}

std::string HTTPResponseBuilder::build() {

  if (status == HTTPStatus::FORBIDDEN) {
    response_body = forbidden_body;
    content_type = HTTPContentType::HTML;
  } else if (status == HTTPStatus::NOT_FOUND) {
    response_body = not_found_body;
    content_type = HTTPContentType::HTML;
  } else if (status == HTTPStatus::BAD_REQUEST) {
    response_body = bad_request_body;
    content_type = HTTPContentType::HTML;
  } else if (status == HTTPStatus::UNSUPPORTED_METHOD) {
    response_body = method_not_allowed_body;
    content_type = HTTPContentType::HTML;
  }

  std::string ct = contenttype_string_map[content_type];

  int content_length = response_body.size();

  // Decide whether the connection should be keep-alive or Close
  // First we check if we got a Connection header from the client
  std::string connection_status = "close";
  if (version == "HTTP/1.1")
    connection_status = "keep-alive";
  if (version == "HTTP/1.0")
    connection_status = "close";
  if (http_headers.count("Connection") == 1) {
    if (http_headers["Connection"] == "keep-alive") {
      connection_status = "keep-alive";
    } else {
      connection_status = "close";
    }
  }

  auto current_date = get_rfc7231_date();

  std::map<std::string, std::string> response_headers;
  response_headers["Content-Type"] = ct;
  response_headers["Content-Length"] = std::to_string(content_length);
  response_headers["Connection"] = connection_status;
  response_headers["Server"] = "gigachad-cpp-server by Ojas Maheshwari";
  response_headers["Date"] = current_date;

  // If binary data is to be served then include additional content-disposition
  // header
  if (content_type == HTTPContentType::OCTET_STREAM &&
      http_requested_filename.has_value()) {
    response_headers["Content-Disposition"] =
        std::string("attachment; filename=") + http_requested_filename.value();
  }

  // Convert response headers into string form
  // This should have been a separate function but nvm

  std::string response = version + " " + httpcode_string_map[status] + "\r\n";
  for (const auto &[key, value] : response_headers) {
    response += key + ": " + value + "\r\n";
  }

  response += "\r\n" + response_body;

  return response;
}