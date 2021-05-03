#include "EmojiPickerSettings.hpp"
#include <QApplication>
#include <functional>
#include <locale>

template <typename T>
std::vector<T> readQSettingsArrayToStdVector(
    QSettings& settings, const QString& prefix, std::function<T(QSettings&)> readValue) {
  std::vector<T> data;

  const int size = settings.beginReadArray(prefix);
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);

    data.push_back(readValue(settings));
  }
  settings.endArray();

  return data;
}

template <typename T>
void writeQSettingsArrayFromStdVector(QSettings& settings, const QString& prefix, const std::vector<T>& data,
    std::function<void(QSettings&, const T&)> writeValue) {
  const int size = data.size();
  settings.beginWriteArray(prefix, size);
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);

    writeValue(settings, data[i]);
  }
  settings.endArray();
}

EmojiPickerSettings* EmojiPickerSettings::_snapshot = nullptr;

EmojiPickerSettings& EmojiPickerSettings::snapshot() {
  if (_snapshot == nullptr) {
    _snapshot = new EmojiPickerSettings();
  }

  return *_snapshot;
}

void EmojiPickerSettings::writeDefaultsToDisk() {
  EmojiPickerSettings settings;

  settings.setRecentEmojis(settings.recentEmojis());
  settings.setLocaleKey(settings.localeKey());
  settings.setSkinTonesDisabled(settings.skinTonesDisabled());
  settings.setGendersDisabled(settings.gendersDisabled());
  settings.setOpenAtMouseLocation(settings.openAtMouseLocation());
  settings.setUseSystemQtTheme(settings.useSystemQtTheme());
  settings.setMaxEmojiVersion(settings.maxEmojiVersion());
  settings.setEmojiAliasesIniFilePath(settings.emojiAliasesIniFilePath());
  settings.setCustomQssFilePath(settings.customQssFilePath());

  settings.setActivateWindowBeforeWritingByDefault(settings.activateWindowBeforeWritingByDefault());
  std::vector<std::string> activateWindowBeforeWritingExceptions = settings.activateWindowBeforeWritingExceptions();
  if (activateWindowBeforeWritingExceptions.size() == 0) {
#ifdef __linux__
    activateWindowBeforeWritingExceptions.push_back("code");
    activateWindowBeforeWritingExceptions.push_back("chromium");
#elif _WIN32
    activateWindowBeforeWritingExceptions.push_back("code.exe");
    activateWindowBeforeWritingExceptions.push_back("chrome.exe");
#endif
  }
  settings.setActivateWindowBeforeWritingExceptions(activateWindowBeforeWritingExceptions);

  settings.setCopyEmojiToClipboardAswellByDefault(settings.copyEmojiToClipboardAswellByDefault());
  std::vector<std::string> copyEmojiToClipboardAswellExceptions = settings.copyEmojiToClipboardAswellExceptions();
  if (copyEmojiToClipboardAswellExceptions.size() == 0) {
#ifdef __linux__
    copyEmojiToClipboardAswellExceptions.push_back("example");
#elif _WIN32
    copyEmojiToClipboardAswellExceptions.push_back("example.exe");
#endif
  }
  settings.setCopyEmojiToClipboardAswellExceptions(copyEmojiToClipboardAswellExceptions);

  settings.setAliasExactMatching(settings.aliasExactMatching());

  settings.setUseClipboardHackByDefault(settings.useClipboardHackByDefault());
  std::vector<std::string> useClipboardHackExceptions = settings.useClipboardHackExceptions();
  if (useClipboardHackExceptions.size() == 0) {
#ifdef __linux__
    useClipboardHackExceptions.push_back("chatterino");
#elif _WIN32
    useClipboardHackExceptions.push_back("chatterino.exe");
#endif
  }
  settings.setUseClipboardHackExceptions(useClipboardHackExceptions);
}

EmojiPickerSettings::EmojiPickerSettings(QObject* parent)
    : QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(),
          QApplication::applicationName(), parent) {
}

std::vector<Emoji> EmojiPickerSettings::recentEmojis() {
  return readQSettingsArrayToStdVector<Emoji>(*this, "recentEmojis", [](QSettings& settings) -> Emoji {
    return {
        settings.value("emojiKey").toString().toStdString(),
        settings.value("emojiStr").toString().toStdString(),
    };
  });
}
void EmojiPickerSettings::setRecentEmojis(const std::vector<Emoji>& recentEmojis) {
  writeQSettingsArrayFromStdVector<Emoji>(
      *this, "recentEmojis", recentEmojis, [](QSettings& settings, const Emoji& emoji) -> void {
        settings.setValue("emojiKey", QString::fromStdString(emoji.name));
        settings.setValue("emojiStr", QString::fromStdString(emoji.code));
      });
}

std::string EmojiPickerSettings::localeKey() const {
  std::string localeKey = value("localeKey", "").toString().toStdString();
  if (localeKey == "system") {
    localeKey = std::locale("").name().substr(0, 2);
  }
  return localeKey;
}
void EmojiPickerSettings::setLocaleKey(const std::string& localeKey) {
  setValue("localeKey", QString::fromStdString(localeKey));
}

bool EmojiPickerSettings::skinTonesDisabled() const {
  return value("skinTonesDisabled", false).toBool();
}
void EmojiPickerSettings::setSkinTonesDisabled(bool skinTonesDisabled) {
  setValue("skinTonesDisabled", skinTonesDisabled);
}

bool EmojiPickerSettings::gendersDisabled() const {
  return value("gendersDisabled", false).toBool();
}
void EmojiPickerSettings::setGendersDisabled(bool gendersDisabled) {
  setValue("gendersDisabled", gendersDisabled);
}

bool EmojiPickerSettings::openAtMouseLocation() const {
  return value("openAtMouseLocation", false).toBool();
}
void EmojiPickerSettings::setOpenAtMouseLocation(bool openAtMouseLocation) {
  setValue("openAtMouseLocation", openAtMouseLocation);
}

bool EmojiPickerSettings::useSystemQtTheme() const {
#ifdef __linux__
  return value("useSystemQtTheme", false).toBool();
#elif _WIN32
  return false;
#endif
}
void EmojiPickerSettings::setUseSystemQtTheme(bool useSystemQtTheme) {
#ifdef __linux__
  setValue("useSystemQtTheme", useSystemQtTheme);
#endif
}

int EmojiPickerSettings::maxEmojiVersion() const {
  return value("maxEmojiVersion", -1).toInt();
}
void EmojiPickerSettings::setMaxEmojiVersion(int maxEmojiVersion) {
  setValue("maxEmojiVersion", maxEmojiVersion);
}

std::string EmojiPickerSettings::emojiAliasesIniFilePath() const {
  return value("emojiAliasesIniFilePath", ":/aliases/github-emojis.ini").toString().toStdString();
}
void EmojiPickerSettings::setEmojiAliasesIniFilePath(const std::string& emojiAliasesIniFilePath) {
  setValue("emojiAliasesIniFilePath", QString::fromStdString(emojiAliasesIniFilePath));
}
std::vector<Emoji> EmojiPickerSettings::aliasedEmojis() {
  std::vector<Emoji> result;

  if (emojiAliasesIniFilePath() == "") {
    return result;
  }

  QSettings emojiAliasesIni{QString::fromStdString(emojiAliasesIniFilePath()), QSettings::IniFormat};

  emojiAliasesIni.beginGroup("Aliases");
  for (const auto& alias : emojiAliasesIni.allKeys()) {
    result.push_back({alias.toStdString(), emojiAliasesIni.value(alias).toString().toStdString()});
  }
  emojiAliasesIni.endGroup();

  return result;
}

std::string EmojiPickerSettings::customQssFilePath() const {
  return value("customQssFilePath", "").toString().toStdString();
}
void EmojiPickerSettings::setCustomQssFilePath(const std::string& customQssFilePath) {
  setValue("customQssFilePath", QString::fromStdString(customQssFilePath));
}

bool EmojiPickerSettings::activateWindowBeforeWritingByDefault() const {
#ifdef __linux__
  return value("activateWindowBeforeWritingByDefault", false).toBool();
#elif _WIN32
  return true;
#endif
}
void EmojiPickerSettings::setActivateWindowBeforeWritingByDefault(bool activateWindowBeforeWritingByDefault) {
#ifdef __linux__
  setValue("activateWindowBeforeWritingByDefault", activateWindowBeforeWritingByDefault);
#endif
}

std::vector<std::string> EmojiPickerSettings::activateWindowBeforeWritingExceptions() {
#ifdef __linux__
  return readQSettingsArrayToStdVector<std::string>(
      *this, "activateWindowBeforeWritingExceptions", [](QSettings& settings) -> std::string {
        return settings.value("processName").toString().toStdString();
      });
#elif _WIN32
  return {};
#endif
}
void EmojiPickerSettings::setActivateWindowBeforeWritingExceptions(
    const std::vector<std::string>& activateWindowBeforeWritingExceptions) {
#ifdef __linux__
  writeQSettingsArrayFromStdVector<std::string>(*this, "activateWindowBeforeWritingExceptions",
      activateWindowBeforeWritingExceptions, [](QSettings& settings, const std::string& exception) -> void {
        settings.setValue("processName", QString::fromStdString(exception));
      });
#endif
}

bool EmojiPickerSettings::copyEmojiToClipboardAswellByDefault() const {
  return value("copyEmojiToClipboardAswellByDefault", false).toBool();
}
void EmojiPickerSettings::setCopyEmojiToClipboardAswellByDefault(bool copyEmojiToClipboardAswellByDefault) {
  setValue("copyEmojiToClipboardAswellByDefault", copyEmojiToClipboardAswellByDefault);
}

std::vector<std::string> EmojiPickerSettings::copyEmojiToClipboardAswellExceptions() {
  return readQSettingsArrayToStdVector<std::string>(
      *this, "copyEmojiToClipboardAswellExceptions", [](QSettings& settings) -> std::string {
        return settings.value("processName").toString().toStdString();
      });
}
void EmojiPickerSettings::setCopyEmojiToClipboardAswellExceptions(
    const std::vector<std::string>& copyEmojiToClipboardAswellExceptions) {
  writeQSettingsArrayFromStdVector<std::string>(*this, "copyEmojiToClipboardAswellExceptions",
      copyEmojiToClipboardAswellExceptions, [](QSettings& settings, const std::string& exception) -> void {
        settings.setValue("processName", QString::fromStdString(exception));
      });
}

bool EmojiPickerSettings::aliasExactMatching() const {
  return value("aliasExactMatching", false).toBool();
}
void EmojiPickerSettings::setAliasExactMatching(bool aliasExactMatching) {
  setValue("aliasExactMatching", aliasExactMatching);
}

bool EmojiPickerSettings::useClipboardHackByDefault() const {
  return value("useClipboardHackByDefault", false).toBool();
}
void EmojiPickerSettings::setUseClipboardHackByDefault(bool useClipboardHackByDefault) {
  setValue("useClipboardHackByDefault", useClipboardHackByDefault);
}

std::vector<std::string> EmojiPickerSettings::useClipboardHackExceptions() {
  return readQSettingsArrayToStdVector<std::string>(
      *this, "useClipboardHackExceptions", [](QSettings& settings) -> std::string {
        return settings.value("processName").toString().toStdString();
      });
}
void EmojiPickerSettings::setUseClipboardHackExceptions(const std::vector<std::string>& useClipboardHackExceptions) {
  writeQSettingsArrayFromStdVector<std::string>(*this, "useClipboardHackExceptions",
      useClipboardHackExceptions, [](QSettings& settings, const std::string& exception) -> void {
        settings.setValue("processName", QString::fromStdString(exception));
      });
}
