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

EmojiPickerSettings::EmojiPickerSettings(QObject* parent)
    : QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(),
          QApplication::applicationName(), parent) {
}

EmojiPickerSettings::~EmojiPickerSettings() {
  setRecentEmojis(recentEmojis());
  setLocaleKey(localeKey());
  setSkinTonesDisabled(skinTonesDisabled());
  setGendersDisabled(gendersDisabled());
  setOpenAtMouseLocation(openAtMouseLocation());
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

std::string EmojiPickerSettings::localeKey() {
  std::string localeKey = value("localeKey", "").toString().toStdString();
  if (localeKey == "system") {
    localeKey = std::locale("").name().substr(0, 2);
  }
  return localeKey;
}
void EmojiPickerSettings::setLocaleKey(const std::string& localeKey) {
  setValue("localeKey", QString::fromStdString(localeKey));
}

bool EmojiPickerSettings::skinTonesDisabled() {
  return value("skinTonesDisabled", false).toBool();
}
void EmojiPickerSettings::setSkinTonesDisabled(bool skinTonesDisabled) {
  setValue("skinTonesDisabled", skinTonesDisabled);
}

bool EmojiPickerSettings::gendersDisabled() {
  return value("gendersDisabled", false).toBool();
}
void EmojiPickerSettings::setGendersDisabled(bool gendersDisabled) {
  setValue("gendersDisabled", gendersDisabled);
}

bool EmojiPickerSettings::openAtMouseLocation() {
  return value("openAtMouseLocation", false).toBool();
}
void EmojiPickerSettings::setOpenAtMouseLocation(bool openAtMouseLocation) {
  setValue("openAtMouseLocation", openAtMouseLocation);
}
