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

const EmojiPickerSettings* EmojiPickerSettings::_startupSnapshot = nullptr;

const EmojiPickerSettings& EmojiPickerSettings::startupSnapshot() {
  if (_startupSnapshot == nullptr) {
    _startupSnapshot = new EmojiPickerSettings();
  }

  return *_startupSnapshot;
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

  std::vector<std::pair<std::string, std::string>> aliases = settings.aliases();
  if (aliases.size() == 0) {
    aliases.emplace_back("rage", "pouting_face");
  }
  settings.setAliases(aliases);

  settings.setCustomQssFilePath(settings.customQssFilePath());
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

std::vector<std::pair<std::string, std::string>> EmojiPickerSettings::aliases() {
  return readQSettingsArrayToStdVector<std::pair<std::string, std::string>>(
      *this, "aliases", [](QSettings& settings) -> std::pair<std::string, std::string> {
        return {
            settings.value("alias").toString().toStdString(),
            settings.value("emojiKey").toString().toStdString(),
        };
      });
}
void EmojiPickerSettings::setAliases(const std::vector<std::pair<std::string, std::string>>& aliases) {
  writeQSettingsArrayFromStdVector<std::pair<std::string, std::string>>(
      *this, "aliases", aliases, [](QSettings& settings, const std::pair<std::string, std::string>& alias) -> void {
        settings.setValue("alias", QString::fromStdString(alias.first));
        settings.setValue("emojiKey", QString::fromStdString(alias.second));
      });
}

std::string EmojiPickerSettings::customQssFilePath() const {
  return value("customQssFilePath", "").toString().toStdString();
}
void EmojiPickerSettings::setCustomQssFilePath(const std::string& customQssFilePath) {
  setValue("customQssFilePath", QString::fromStdString(customQssFilePath));
}
