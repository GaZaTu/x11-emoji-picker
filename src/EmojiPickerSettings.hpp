#pragma once

#include "emojis.hpp"
#include <QSettings>
#include <utility>
#include <vector>

class EmojiPickerSettings : public QSettings {
  Q_OBJECT

public:
  static EmojiPickerSettings& snapshot();

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

  std::string emojiAliasesIniFilePath() const;
  void setEmojiAliasesIniFilePath(const std::string& emojiAliasesIniFilePath);
  std::vector<Emoji> aliasedEmojis();

  std::string customQssFilePath() const;
  void setCustomQssFilePath(const std::string& customQssFilePath);

  bool activateWindowBeforeWritingByDefault() const;
  void setActivateWindowBeforeWritingByDefault(bool activateWindowBeforeWritingByDefault);

  std::vector<std::string> activateWindowBeforeWritingExceptions();
  void setActivateWindowBeforeWritingExceptions(const std::vector<std::string>& activateWindowBeforeWritingExceptions);

  bool copyEmojiToClipboardAswellByDefault() const;
  void setCopyEmojiToClipboardAswellByDefault(bool copyEmojiToClipboardAswellByDefault);

  std::vector<std::string> copyEmojiToClipboardAswellExceptions();
  void setCopyEmojiToClipboardAswellExceptions(const std::vector<std::string>& copyEmojiToClipboardAswellExceptions);

  bool aliasExactMatching() const;
  void setAliasExactMatching(bool aliasExactMatching);

  bool useClipboardHackByDefault() const;
  void setUseClipboardHackByDefault(bool useClipboardHackByDefault);

  std::vector<std::string> useClipboardHackExceptions();
  void setUseClipboardHackExceptions(const std::vector<std::string>& useClipboardHackExceptions);

private:
  static EmojiPickerSettings* _snapshot;
};
