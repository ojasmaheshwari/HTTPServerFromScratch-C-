#include "http_response_builder.h"
#include "http_parser.h"
#include <string>

HTTPResponseBuilder::HTTPResponseBuilder(const std::string &version,
                                         HTTPStatus status,
                                         const std::string &response_body,
                                         HTTPContentType content_type)
    : version(version), status(status), response_body(response_body),
      content_type(content_type) {
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
}

std::string HTTPResponseBuilder::build() {
  std::string ct = contenttype_string_map[content_type];

  if (status == HTTPStatus::FORBIDDEN) {
    response_body = forbidden_body;
  } else if (status == HTTPStatus::NOT_FOUND) {
    response_body = not_found_body;
  } else if (status == HTTPStatus::BAD_REQUEST) {
    response_body = bad_request_body;
  } else if (status == HTTPStatus::UNSUPPORTED_METHOD) {
    response_body = method_not_allowed_body;
  }

  int content_length = response_body.size();

  std::string response = version + " " + httpcode_string_map[status] + "\r\n" +
                         "Content-Type: " + ct + "\r\n" +
                         "Content-Length: " + std::to_string(content_length) +
                         "\r\n" + "Connection: keep-alive" + "\r\n" + "\r\n" +
                         response_body;
  return response;
}