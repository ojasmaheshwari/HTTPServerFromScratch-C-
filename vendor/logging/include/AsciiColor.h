#pragma once

#include <string>

namespace Ascii {
  enum class Color {
    None = 0, Yellow, Red   
  };
}

class AsciiColor {
public:
  static const std::string colorized(const std::string &text, Ascii::Color color);
};