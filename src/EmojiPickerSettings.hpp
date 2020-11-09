#pragma once

#include "emojis.hpp"
#include <QSettings>
#include <utility>
#include <vector>

class EmojiPickerSettings : public QSettings {
  Q_OBJECT

public:
  static const EmojiPickerSettings& startupSnapshot();

  static void writeDefaultsToDisk();

  explicit EmojiPickerSettings(QObject* parent = nullptr);

  std::vector<Emoji> recentEmojis();
  void setRecentEmojis(const std::vector<Emoji>& recentEmojis);

  std::string localeKey() const;
  void setLocaleKey(const std::string& localeKey);

  bool skinTonesDisabled() const;
  void setSkinTonesDisabled(bool skinTonesDisabled);

  bool gendersDisabled() const;
  void setGendersDisabled(bool gendersDisabled);

  bool openAtMouseLocation() const;
  void setOpenAtMouseLocation(bool openAtMouseLocation);

  bool useSystemQtTheme() const;
  void setUseSystemQtTheme(bool useSystemQtTheme);

  int maxEmojiVersion() const;
  void setMaxEmojiVersion(int maxEmojiVersion);

  std::vector<std::pair<std::string, std::string>> aliases();
  void setAliases(const std::vector<std::pair<std::string, std::string>>& aliases);

  std::string customQssFilePath() const;
  void setCustomQssFilePath(const std::string& customQssFilePath);

private:
  static const EmojiPickerSettings* _startupSnapshot;
};
