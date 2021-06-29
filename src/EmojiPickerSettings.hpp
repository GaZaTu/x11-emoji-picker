#pragma once

#include "emojis.hpp"
#include <QSettings>
#include <utility>
#include <vector>

class EmojiPickerSettings : public QSettings {
  Q_OBJECT

public:
  static QSettings::Scope snapshotScope;
  static EmojiPickerSettings& snapshot();

  static void writeDefaultsToDisk();

  explicit EmojiPickerSettings(QSettings::Scope scope = snapshotScope, QObject* parent = nullptr);

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

  std::vector<std::string> emojiAliasesIniFilePaths();
  void setEmojiAliasesIniFilePaths(const std::vector<std::string>& emojiAliasesIniFilePaths);
  std::vector<Emoji> aliasedEmojis();

  bool aliasExactMatching() const;
  void setAliasExactMatching(bool aliasExactMatching);

  std::string customQssFilePath() const;
  void setCustomQssFilePath(const std::string& customQssFilePath);

  bool activateWindowBeforeWritingByDefault() const;
  void setActivateWindowBeforeWritingByDefault(bool activateWindowBeforeWritingByDefault);

  std::vector<std::string> activateWindowBeforeWritingExceptions();
  void setActivateWindowBeforeWritingExceptions(const std::vector<std::string>& activateWindowBeforeWritingExceptions);

  bool activateWindowBeforeWriting(const std::string& processName);

  bool useClipboardHackByDefault() const;
  void setUseClipboardHackByDefault(bool useClipboardHackByDefault);

  std::vector<std::string> useClipboardHackExceptions();
  void setUseClipboardHackExceptions(const std::vector<std::string>& useClipboardHackExceptions);

  bool useClipboardHack(const std::string& processName);

  double windowOpacity() const;
  void setWindowOpacity(double windowOpacity);

  bool swapEnterAndShiftEnter() const;
  void setSwapEnterAndShiftEnter(bool swapEnterAndShiftEnter);

  bool surroundAliasesWithColons() const;
  void setSurroundAliasesWithColons(bool surroundAliasesWithColons);

  bool hideInputMethod() const;
  void setHideInputMethod(bool hideInputMethod);

  bool enableEmojiIncludesSearch() const;
  void setEnableEmojiIncludesSearch(bool enableEmojiIncludesSearch);

  void toggleInputMethod(const std::string& processName);

private:
  static EmojiPickerSettings* _snapshot;
};
