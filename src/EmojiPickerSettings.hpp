#pragma once

#include "emojis.hpp"
#include <QSettings>
#include <utility>
#include <vector>
#include <QFontMetrics>
#include <unordered_map>

class EmojiPickerSettings : public QSettings {
  Q_OBJECT

public:
  static EmojiPickerSettings& snapshot();

  static void writeDefaultsToDisk();

  explicit EmojiPickerSettings(QObject* parent = nullptr);

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

  bool isDisabledEmoji(const Emoji& emoji, const QFontMetrics& fontMetrics);

  std::vector<std::string> emojiAliasesIniFilePaths();
  void setEmojiAliasesIniFilePaths(const std::vector<std::string>& emojiAliasesIniFilePaths);

  std::unordered_map<std::string, std::vector<QString>> emojiAliases();

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

  bool useSystemEmojiFont() const;
  void setUseSystemEmojiFont(bool useSystemEmojiFont);

  bool useSystemEmojiFontWidthHeuristics() const;
  void setUseSystemEmojiFontWidthHeuristics(bool useSystemEmojiFontWidthHeuristics);

  bool closeOnFocusLost() const;
  void setCloseOnFocusLost(bool closeOnFocusLost);

  std::string scaleFactor() const;
  void setScaleFactor(std::string scaleFactor);

  bool saveKaomojiInMRU() const;
  void setSaveKaomojiInMRU(bool saveKaomojiInMRU);

  void toggleInputMethod(const std::string& processName);

private:
  static EmojiPickerSettings* _snapshot;
};

class EmojiPickerCache : public QSettings {
  Q_OBJECT

public:
  explicit EmojiPickerCache();

  ~EmojiPickerCache();

  std::vector<Emoji> emojiMRU();
  void emojiMRU(const std::vector<Emoji>& emojiMRU);

private:
  static QString path();
};
