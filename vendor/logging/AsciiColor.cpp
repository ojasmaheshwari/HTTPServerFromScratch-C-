#include "include/AsciiColor.h"

#include <format>

const std::string AsciiColor::colorized(const std::string &text, Ascii::Color color) {
  switch(color) {
    case Ascii::Color::None:
      return text;
      break;
    case Ascii::Color::Yellow:
      return std::format("{}{}{}", "\033[1;33m", text, "\033[0m\n");
      break;
    case Ascii::Color::Red:
      return std::format("{}{}{}", "\033[1;31m", text, "\033[0m\n");
      break;
    default:
      return text;
      break;
  }
}