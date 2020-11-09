#include "EmojiLabel.hpp"
#include "EmojiPickerSettings.hpp"
#include <sstream>
#include <unicode/schriter.h>
#include <unicode/unistr.h>

EmojiLabel::EmojiLabel(QWidget* parent) : QLabel(parent) {
  setGraphicsEffect(_shadowEffect);
  setMouseTracking(true);

  if (EmojiPickerSettings::startupSnapshot().useSystemQtTheme()) {
    _shadowEffect->setColor(palette().text().color());
  } else {
    _shadowEffect->setColor(QColor(240, 240, 240));
  }

  _shadowEffect->setOffset(0);
  _shadowEffect->setBlurRadius(20);
  _shadowEffect->setEnabled(false);
}

EmojiLabel::EmojiLabel(QWidget* parent, const Emoji& emoji) : EmojiLabel(parent) {
  setEmoji(emoji);
}

// fdm `:/res/72x72/${[...emojiStr].map(c => c.codePointAt(0).toString(16)).join('-')}.png`
std::string getPixmapPathByEmojiStr(const std::string& emojiStr) {
  std::stringstream emojiHexCodeStream;

  emojiHexCodeStream << ":/res/72x72/";

  bool firstCodepoint = true;
  icu::UnicodeString emojiUStr(emojiStr.data(), emojiStr.length(), "utf-8");
  icu::StringCharacterIterator emojiUStrIterator(emojiUStr);
  while (emojiUStrIterator.hasNext()) {
    int32_t codepoint = emojiUStrIterator.next32PostInc();

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

const Emoji& EmojiLabel::emoji() {
  return _emoji;
}
void EmojiLabel::setEmoji(const Emoji& emoji, int w, int h) {
  _emoji = emoji;

  setAccessibleName(QString::fromStdString(_emoji.name));
  setPixmap(getPixmapByEmojiStr(_emoji.code).scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

bool EmojiLabel::highlighted() {
  return _shadowEffect->isEnabled();
}
void EmojiLabel::setHighlighted(bool highlighted) {
  _shadowEffect->setEnabled(highlighted);
}

void EmojiLabel::mousePressEvent(QMouseEvent* ev) {
  emit mousePressed(ev);

  QLabel::mousePressEvent(ev);
}

void EmojiLabel::mouseMoveEvent(QMouseEvent* ev) {
  if (toolTip().isNull()) {
    setToolTip(tr(_emoji.name.data()));
  }

  QLabel::mouseMoveEvent(ev);
}
