#pragma once

#include <string>

struct Kaomoji {
public:
  std::string name;
  std::string text;

  bool operator==(const Kaomoji& other) const {
    return name == other.name;
  }

  operator bool() const {
    return text != "";
  }
};

extern const Kaomoji kaomojis[293];
