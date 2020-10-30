#pragma once

#include "emojis.hpp"
#include <QSettings>
#include <vector>

class EmojiPickerSettings : public QSettings {
  Q_OBJECT

public:
  std::vector<Emoji> recentEmojis();
  void setRecentEmojis(const std::vector<Emoji>& recentEmojis);

  std::string localeKey();
  void setLocaleKey(const std::string& localeKey);
};
