#pragma once

#include "emojis.hpp"
#include <QTranslator>
#include <unordered_map>

class EmojiTranslator : public QTranslator {
  Q_OBJECT
public:
  explicit EmojiTranslator(
      QObject* parent = nullptr, const std::string& localeKey = std::locale("").name().substr(0, 2));

  virtual QString translate(
      const char* context, const char* sourceText, const char* disambiguation = nullptr, int n = -1) const;

private:
  std::string _localeKey;

  std::unordered_map<std::string, Emoji> _emojis;

  void fillEmojisMap();
};
