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

 contenttype_string_map[HTTPContentType::HTML] = "text/html";
 contenttype_string_map[HTTPContentType::PNG] = "image/png";
 contenttype_string_map[HTTPContentType::JPG] = "image/jpg";
 contenttype_string_map[HTTPContentType::JPEG] = "image/jpeg";
 contenttype_string_map[HTTPContentType::GIF] = "image/gif";
}

std::string HTTPResponseBuilder::build() {
  std::string ct = contenttype_string_map[content_type];

  int content_length = response_body.size();

  std::string response = version + " " + httpcode_string_map[status] + "\r\n" +
                         "Content-Type: " + ct + "\r\n" +
                         "Content-Length: " + std::to_string(content_length) +
                         "\r\n" + "Connection: Close" + "\r\n" + "\r\n" +
                         response_body;
  return response;
}