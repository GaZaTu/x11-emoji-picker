#include "EmojiLabel.hpp"
#include <sstream>
#include <unicode/schriter.h>
#include <unicode/unistr.h>

EmojiLabel::EmojiLabel(QWidget* parent) : QLabel(parent) {
  setGraphicsEffect(&_shadowEffect);

  _shadowEffect.setColor(QColor(255, 255, 255, 255));
  _shadowEffect.setOffset(0);
  _shadowEffect.setBlurRadius(20);
  _shadowEffect.setEnabled(false);
}

EmojiLabel::EmojiLabel(QWidget* parent, const std::string& emojiStr) : EmojiLabel(parent) {
  setEmojiStr(emojiStr);
}

// fdm `:/res/72x72/${[...emojiStr].map(c => c.toString(16)).join('-')}.png`
std::string getPixmapPathByEmojiStr(const std::string& emojiStr) {
  std::stringstream emojiHexCodeStream;

  emojiHexCodeStream << ":/res/72x72/";

  bool firstCodepoint = true;
  icu::UnicodeString emojiUStr(emojiStr.data(), emojiStr.length(), "utf-8");
  icu::StringCharacterIterator emojiUStrIterator(emojiUStr);
  while (emojiUStrIterator.hasNext()) {
    UChar32 codepoint = emojiUStrIterator.next32PostInc();

    if (firstCodepoint) {
      firstCodepoint = false;
    } else {
      emojiHexCodeStream << "-";
    }

    emojiHexCodeStream << std::hex << codepoint;
  }

  emojiHexCodeStream << ".png";

  return emojiHexCodeStream.str();
}

QPixmap getPixmapByEmojiStr(const std::string& emojiStr) {
  return QPixmap(QString::fromStdString(getPixmapPathByEmojiStr(emojiStr)), "PNG");
}

const std::string& EmojiLabel::emojiStr() {
  return _emojiStr;
}
void EmojiLabel::setEmojiStr(const std::string& emojiStr) {
  _emojiStr = emojiStr;

  setAccessibleName(QString::fromStdString(emojiStr));
  setPixmap(getPixmapByEmojiStr(_emojiStr).scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

bool EmojiLabel::highlighted() {
  return _shadowEffect.isEnabled();
}
void EmojiLabel::setHighlighted(bool highlighted) {
  _shadowEffect.setEnabled(highlighted);
}
