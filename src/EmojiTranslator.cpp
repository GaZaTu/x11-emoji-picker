#include "EmojiTranslator.hpp"

const std::string skinToneSuffix = "_skin_tone";
const std::string skinToneSuffixes[] = {
    "_medium" + skinToneSuffix,
    "_medium_light" + skinToneSuffix,
    "_medium_dark" + skinToneSuffix,
    "_light" + skinToneSuffix,
    "_dark" + skinToneSuffix,
};

const std::string personPrefix = "person_";
const std::string personPrefixes[] = {
    "man_",
    "woman_",
};

const std::string peoplePrefix = "people_";
const std::string peoplePrefixes[] = {
    "men_",
    "women_",
};

const std::string personSuffixes[] = {
    "_man",
    "_woman",
    "_boy",
    "_girl",
};

EmojiTranslator::EmojiTranslator(QObject* parent, const std::string& localeKey)
    : QTranslator(parent), _localeKey(localeKey) {
  fillEmojisMap();
}

void EmojiTranslator::fillEmojisMap() {
  if (_localeKey == "none") {
    for (const auto& emoji : emojis) {
      _emojis[emoji.name] = emoji;
    }

    return;
  }

  for (const auto& emoji : emojis) {
    _emojis[emoji.name] = emoji;

    if (emoji.name.find(skinToneSuffix) != std::string::npos) {
      std::string nameWithoutSkinTone = emoji.name;

      for (const auto& skinToneSuffix : skinToneSuffixes) {
        int idx = 0;

        while ((idx = nameWithoutSkinTone.find(skinToneSuffix)) != std::string::npos) {
          nameWithoutSkinTone =
              QString::fromStdString(nameWithoutSkinTone).replace(idx, skinToneSuffix.length(), "").toStdString();
        }
      }

      auto emojiWithoutSkinTone = _emojis.find(nameWithoutSkinTone);
      if (emojiWithoutSkinTone != _emojis.end()) {
        _emojis[emoji.name] = emojiWithoutSkinTone->second;
      }
    }

    for (const auto& genderPrefix : personPrefixes) {
      int idx = emoji.name.find(genderPrefix);

      if (idx == 0) {
        auto nameWithoutGender = personPrefix + emoji.name.substr(genderPrefix.length());
        auto emojiWithoutGender = _emojis.find(nameWithoutGender);

        if (emojiWithoutGender == _emojis.end()) {
          nameWithoutGender = emoji.name.substr(genderPrefix.length());
          emojiWithoutGender = _emojis.find(nameWithoutGender);
        }

        if (emojiWithoutGender != _emojis.end()) {
          _emojis[emoji.name] = emojiWithoutGender->second;
        }

        break;
      }
    }

    for (const auto& genderPrefix : peoplePrefixes) {
      int idx = emoji.name.find(genderPrefix);

      if (idx == 0) {
        auto nameWithoutGender = peoplePrefix + emoji.name.substr(genderPrefix.length());
        auto emojiWithoutGender = _emojis.find(nameWithoutGender);

        if (emojiWithoutGender == _emojis.end()) {
          nameWithoutGender = emoji.name.substr(genderPrefix.length());
          emojiWithoutGender = _emojis.find(nameWithoutGender);
        }

        if (emojiWithoutGender != _emojis.end()) {
          _emojis[emoji.name] = emojiWithoutGender->second;
        }

        break;
      }
    }

    std::string nameWithoutPersonSuffixes = emoji.name;

    for (const auto& personSuffix : personSuffixes) {
      int idx = 0;

      while ((idx = nameWithoutPersonSuffixes.find(personSuffix)) != std::string::npos) {
        nameWithoutPersonSuffixes =
            QString::fromStdString(nameWithoutPersonSuffixes).replace(idx, personSuffix.length(), "").toStdString();
      }
    }

    auto emojiWithoutPersonSuffixes = _emojis.find(nameWithoutPersonSuffixes);
    if (emojiWithoutPersonSuffixes != _emojis.end()) {
      _emojis[emoji.name] = emojiWithoutPersonSuffixes->second;
    }
  }
}

QString EmojiTranslator::translate(
    const char* context, const char* sourceText, const char* disambiguation, int n) const {
  auto found = _emojis.find(sourceText);
  if (found == _emojis.end()) {
    return QString();
  }

  return QString::fromStdString(found->second.nameByLocale(_localeKey));
}
