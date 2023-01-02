#pragma once

#include <locale>
#include <string>

struct Emoji {
public:
  std::string name;
  std::string code;
  short version = -1;

  std::string nameByLocale(const std::string& localeKey = std::locale("").name().substr(0, 2)) const;

  bool isGenderVariation() const;

  bool isSkinToneVariation() const;

  bool operator==(const Emoji& other) const;

  operator bool() const;

private:
  template <int LOCALE_ID>
  std::string nameByLocaleId() const;

  static constexpr unsigned int hashLocaleKey(const char* s, int off = 0) {
    return !s[off] ? 5381 : (hashLocaleKey(s, off + 1) * 33) ^ s[off];
  }
};

// generated from https://unicode.org/Public/emoji/14.0/emoji-test.txt
extern const Emoji emojis[3624];
