#include "EmojiPickerSettings.hpp"
#include <QApplication>
#include <QStandardPaths>
#include <algorithm>
#include <functional>
#include <locale>

template <typename T>
bool vectorIncludes(const std::vector<T>& vector, const T& value) {
  return std::find(vector.begin(), vector.end(), value) != vector.end();
}

template <typename T>
void vectorRemove(std::vector<T>& vector, const T& value) {
  vector.erase(std::remove(vector.begin(), vector.end(), value), vector.end());
}

template <typename T>
std::vector<T> readQSettingsArrayToStdVector(QSettings& settings, const QString& prefix,
    std::function<T(QSettings&)> readValue, const std::vector<T>& defaultValue = {}) {
  std::vector<T> data;

  const int size = settings.beginReadArray(prefix);
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);

    data.push_back(readValue(settings));
  }
  settings.endArray();

  if (data.size() == 0) {
    return defaultValue;
  }

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
  settings.setEmojiAliasesIniFilePaths(settings.emojiAliasesIniFilePaths());
  settings.setAliasExactMatching(settings.aliasExactMatching());
  settings.setCustomQssFilePath(settings.customQssFilePath());
  settings.setActivateWindowBeforeWritingByDefault(settings.activateWindowBeforeWritingByDefault());
  settings.setActivateWindowBeforeWritingExceptions(settings.activateWindowBeforeWritingExceptions());
  settings.setUseClipboardHackByDefault(settings.useClipboardHackByDefault());
  settings.setUseClipboardHackExceptions(settings.useClipboardHackExceptions());
  settings.setWindowOpacity(settings.windowOpacity());
  settings.setSwapEnterAndShiftEnter(settings.swapEnterAndShiftEnter());
  settings.setSurroundAliasesWithColons(settings.surroundAliasesWithColons());
  settings.setHideInputMethod(settings.hideInputMethod());
  settings.setEnableEmojiIncludesSearch(settings.enableEmojiIncludesSearch());
  settings.setUseSystemEmojiFont(settings.useSystemEmojiFont());
  settings.setUseSystemEmojiFontWidthHeuristics(settings.useSystemEmojiFontWidthHeuristics());
  settings.setStartInKaomojiMode(settings.startInKaomojiMode());
  settings.setCloseOnFocusLost(settings.closeOnFocusLost());
  settings.setPreviewTextLeftMargin(settings.previewTextLeftMargin());
}

EmojiPickerSettings::EmojiPickerSettings(QObject* parent)
    : QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(),
          QApplication::applicationName(), parent) {
}

class EmojiPickerCache : public QSettings {
public:
  EmojiPickerCache() : QSettings(path(), QSettings::IniFormat) {
  }

  ~EmojiPickerCache() {
    setValue("version", QApplication::applicationVersion());
  }

private:
  static QString path() {
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/cache.ini";
  }
};

std::vector<Emoji> EmojiPickerSettings::recentEmojis() {
  auto prefix = "recentEmojis";
  auto handler = [](QSettings& settings) -> Emoji {
    return {
        settings.value("emojiKey").toString().toStdString(),
        settings.value("emojiStr").toString().toStdString(),
    };
  };

  EmojiPickerCache cache;

  auto recentEmojis = readQSettingsArrayToStdVector<Emoji>(cache, prefix, handler);
  if (recentEmojis.empty()) {
    recentEmojis = readQSettingsArrayToStdVector<Emoji>(*this, prefix, handler);
  }

  return recentEmojis;
}
void EmojiPickerSettings::setRecentEmojis(const std::vector<Emoji>& recentEmojis) {
  auto prefix = "recentEmojis";
  auto handler = [](QSettings& settings, const Emoji& emoji) -> void {
    settings.setValue("emojiKey", QString::fromStdString(emoji.name));
    settings.setValue("emojiStr", QString::fromStdString(emoji.code));
  };

  EmojiPickerCache cache;

  writeQSettingsArrayFromStdVector<Emoji>(cache, prefix, recentEmojis, handler);
  remove(prefix);
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

std::vector<std::string> defaultEmojiAliasesIniFilePaths = {
    ":/aliases/github-emojis.ini",
    ":/aliases/gitmoji-emojis.ini",
};

std::vector<std::string> EmojiPickerSettings::emojiAliasesIniFilePaths() {
  return readQSettingsArrayToStdVector<std::string>(
      *this, "emojiAliasesIniFilePaths",
      [](QSettings& settings) -> std::string {
        return settings.value("path").toString().toStdString();
      },
      defaultEmojiAliasesIniFilePaths);
}
void EmojiPickerSettings::setEmojiAliasesIniFilePaths(const std::vector<std::string>& emojiAliasesIniFilePaths) {
  writeQSettingsArrayFromStdVector<std::string>(*this, "emojiAliasesIniFilePaths", emojiAliasesIniFilePaths,
      [](QSettings& settings, const std::string& exception) -> void {
        settings.setValue("path", QString::fromStdString(exception));
      });
}
std::vector<Emoji> EmojiPickerSettings::aliasedEmojis() {
  std::vector<std::string> paths = emojiAliasesIniFilePaths();
  std::vector<Emoji> result;

  for (const std::string& path : paths) {
    QSettings emojiAliasesIni{QString::fromStdString(path), QSettings::IniFormat};

    emojiAliasesIni.beginGroup("AliasesMap");
    for (const auto& alias : emojiAliasesIni.allKeys()) {
      auto key = alias.toStdString();
      auto str = emojiAliasesIni.value(alias).toString().toStdString();

      if (surroundAliasesWithColons()) {
        key = ":" + key + ":";
      }

      result.push_back({key, str});
    }
    emojiAliasesIni.endGroup();

    int arraySize = emojiAliasesIni.beginReadArray("AliasesList");
    for (int i = 0; i < arraySize; i++) {
      emojiAliasesIni.setArrayIndex(i);

      auto key = emojiAliasesIni.value("emojiKey").toString().toStdString();
      auto str = emojiAliasesIni.value("emojiStr").toString().toStdString();

      if (surroundAliasesWithColons()) {
        key = ":" + key + ":";
      }

      result.push_back({key, str});
    }
    emojiAliasesIni.endArray();
  }

  return result;
}

bool EmojiPickerSettings::aliasExactMatching() const {
  return value("aliasExactMatching", false).toBool();
}
void EmojiPickerSettings::setAliasExactMatching(bool aliasExactMatching) {
  setValue("aliasExactMatching", aliasExactMatching);
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

std::vector<std::string> defaultActivateWindowBeforeWritingExceptions = {
    "code",
    "code-oss",
    "chrome",
    "chromium",
    "kate",
    "brave",
};

std::vector<std::string> EmojiPickerSettings::activateWindowBeforeWritingExceptions() {
#ifdef __linux__
  return readQSettingsArrayToStdVector<std::string>(
      *this, "activateWindowBeforeWritingExceptions",
      [](QSettings& settings) -> std::string {
        return settings.value("processName").toString().toStdString();
      },
      defaultActivateWindowBeforeWritingExceptions);
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

bool EmojiPickerSettings::activateWindowBeforeWriting(const std::string& processName) {
  bool isDefault = activateWindowBeforeWritingByDefault();
  auto exceptions = activateWindowBeforeWritingExceptions();
  bool isException = vectorIncludes(exceptions, processName);

  return ((isDefault && !isException) || (!isDefault && isException));
}

bool EmojiPickerSettings::useClipboardHackByDefault() const {
  return value("useClipboardHackByDefault", false).toBool();
}
void EmojiPickerSettings::setUseClipboardHackByDefault(bool useClipboardHackByDefault) {
  setValue("useClipboardHackByDefault", useClipboardHackByDefault);
}

std::vector<std::string> defaultUseClipboardHackExceptions = {
    "chatterino",
    "kate",
};

std::vector<std::string> EmojiPickerSettings::useClipboardHackExceptions() {
  return readQSettingsArrayToStdVector<std::string>(
      *this, "useClipboardHackExceptions",
      [](QSettings& settings) -> std::string {
        return settings.value("processName").toString().toStdString();
      },
      defaultUseClipboardHackExceptions);
}
void EmojiPickerSettings::setUseClipboardHackExceptions(const std::vector<std::string>& useClipboardHackExceptions) {
  writeQSettingsArrayFromStdVector<std::string>(*this, "useClipboardHackExceptions", useClipboardHackExceptions,
      [](QSettings& settings, const std::string& exception) -> void {
        settings.setValue("processName", QString::fromStdString(exception));
      });
}

bool EmojiPickerSettings::useClipboardHack(const std::string& processName) {
  bool isDefault = useClipboardHackByDefault();
  auto exceptions = useClipboardHackExceptions();
  bool isException = vectorIncludes(exceptions, processName);

  return ((isDefault && !isException) || (!isDefault && isException));
}

double EmojiPickerSettings::windowOpacity() const {
  return value("windowOpacity", 0.90).toDouble();
}
void EmojiPickerSettings::setWindowOpacity(double windowOpacity) {
  setValue("windowOpacity", windowOpacity);
}

bool EmojiPickerSettings::swapEnterAndShiftEnter() const {
#ifdef __linux__
  return value("swapEnterAndShiftEnter", false).toBool();
#elif _WIN32
  return true;
#endif
}
void EmojiPickerSettings::setSwapEnterAndShiftEnter(bool swapEnterAndShiftEnter) {
  setValue("swapEnterAndShiftEnter", swapEnterAndShiftEnter);
}

bool EmojiPickerSettings::surroundAliasesWithColons() const {
  return value("surroundAliasesWithColons", true).toBool();
}
void EmojiPickerSettings::setSurroundAliasesWithColons(bool surroundAliasesWithColons) {
  setValue("surroundAliasesWithColons", surroundAliasesWithColons);
}

bool EmojiPickerSettings::hideInputMethod() const {
  return value("hideInputMethod", false).toBool();
}
void EmojiPickerSettings::setHideInputMethod(bool hideInputMethod) {
  setValue("hideInputMethod", hideInputMethod);
}

bool EmojiPickerSettings::enableEmojiIncludesSearch() const {
  return value("enableEmojiIncludesSearch", true).toBool();
}
void EmojiPickerSettings::setEnableEmojiIncludesSearch(bool enableEmojiIncludesSearch) {
  setValue("enableEmojiIncludesSearch", enableEmojiIncludesSearch);
}

bool EmojiPickerSettings::useSystemEmojiFont() const {
  return value("useSystemEmojiFont", false).toBool();
}
void EmojiPickerSettings::setUseSystemEmojiFont(bool useSystemEmojiFont) {
  setValue("useSystemEmojiFont", useSystemEmojiFont);
}

bool EmojiPickerSettings::useSystemEmojiFontWidthHeuristics() const {
  return value("useSystemEmojiFontWidthHeuristics", true).toBool();
}
void EmojiPickerSettings::setUseSystemEmojiFontWidthHeuristics(bool useSystemEmojiFontWidthHeuristics) {
  setValue("useSystemEmojiFontWidthHeuristics", useSystemEmojiFontWidthHeuristics);
}

bool EmojiPickerSettings::startInKaomojiMode() const {
  return value("startInKaomojiMode", false).toBool();
}
void EmojiPickerSettings::setStartInKaomojiMode(bool startInKaomojiMode) {
  setValue("startInKaomojiMode", startInKaomojiMode);
}

bool EmojiPickerSettings::closeOnFocusLost() const {
  return value("closeOnFocusLost", false).toBool();
}
void EmojiPickerSettings::setCloseOnFocusLost(bool closeOnFocusLost) {
  setValue("closeOnFocusLost", closeOnFocusLost);
}

int EmojiPickerSettings::previewTextLeftMargin() const {
  return value("previewTextLeftMargin", 0).toInt();
}
void EmojiPickerSettings::setPreviewTextLeftMargin(int previewTextLeftMargin) {
  setValue("previewTextLeftMargin", previewTextLeftMargin);
}

void EmojiPickerSettings::toggleInputMethod(const std::string& processName) {
  auto activateWindowBeforeWritingExceptions = this->activateWindowBeforeWritingExceptions();
  auto useClipboardHackExceptions = this->useClipboardHackExceptions();

  bool isActivateWindowBeforeWritingException = vectorIncludes(activateWindowBeforeWritingExceptions, processName);
  bool isUseClipboardHackException = vectorIncludes(useClipboardHackExceptions, processName);

  if (!isActivateWindowBeforeWritingException && !isUseClipboardHackException) {
    useClipboardHackExceptions.push_back(processName);
    this->setUseClipboardHackExceptions(useClipboardHackExceptions);
  } else if (!isActivateWindowBeforeWritingException && isUseClipboardHackException) {
    vectorRemove(useClipboardHackExceptions, processName);
    this->setUseClipboardHackExceptions(useClipboardHackExceptions);
    activateWindowBeforeWritingExceptions.push_back(processName);
    this->setActivateWindowBeforeWritingExceptions(activateWindowBeforeWritingExceptions);
  } else if (isActivateWindowBeforeWritingException && !isUseClipboardHackException) {
    useClipboardHackExceptions.push_back(processName);
    this->setUseClipboardHackExceptions(useClipboardHackExceptions);
  } else {
    vectorRemove(useClipboardHackExceptions, processName);
    this->setUseClipboardHackExceptions(useClipboardHackExceptions);
    vectorRemove(activateWindowBeforeWritingExceptions, processName);
    this->setActivateWindowBeforeWritingExceptions(activateWindowBeforeWritingExceptions);
  }
}
