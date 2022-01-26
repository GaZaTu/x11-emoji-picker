#include "EmojiLabel.hpp"
#include "EmojiPickerSettings.hpp"
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <sstream>
#include <unicode/schriter.h>
#include <unicode/unistr.h>

EmojiLabel::EmojiLabel(QWidget* parent) : QLabel(parent) {
  setGraphicsEffect(_shadowEffect);
  setMouseTracking(true);

  if (EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    _shadowEffect->setColor(palette().text().color());
  } else {
    _shadowEffect->setColor(QColor(240, 240, 240));
  }

  _shadowEffect->setOffset(0);
  _shadowEffect->setBlurRadius(20);
  _shadowEffect->setEnabled(false);

  _devicePixelRatio = QApplication::primaryScreen()->devicePixelRatio();
}

EmojiLabel::EmojiLabel(QWidget* parent, const Emoji& emoji) : EmojiLabel(parent) {
  setEmoji(emoji);
}

void getCodepointsByEmojiStr(
    const std::string& emojiStr, const std::string& separator, std::stringstream& emojiHexCodeStream) {
  bool firstCodepoint = true;
  icu::UnicodeString emojiUStr(emojiStr.data(), emojiStr.length(), "utf-8");
  icu::StringCharacterIterator emojiUStrIterator(emojiUStr);
  while (emojiUStrIterator.hasNext()) {
    int32_t codepoint = emojiUStrIterator.next32PostInc();

    if (firstCodepoint) {
      firstCodepoint = false;
    } else {
      emojiHexCodeStream << separator;
    }

    emojiHexCodeStream << std::hex << codepoint;
  }
}
std::string getCodepointsByEmojiStr(const std::string& emojiStr, const std::string& separator) {
  std::stringstream emojiHexCodeStream;
  getCodepointsByEmojiStr(emojiStr, separator, emojiHexCodeStream);
  return emojiHexCodeStream.str();
}

// fdm `:/res/72x72/${[...emojiStr].map(c => c.codePointAt(0).toString(16)).join('-')}.png`
std::string getPixmapPathByEmojiStr(const std::string& emojiStr) {
  std::stringstream emojiHexCodeStream;

  emojiHexCodeStream << ":/res/72x72/";
  getCodepointsByEmojiStr(emojiStr, "-", emojiHexCodeStream);
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

  w = w * _devicePixelRatio;
  h = h * _devicePixelRatio;

  setAccessibleName(QString::fromStdString(_emoji.name));

  QPixmap emojiPixmap = getPixmapByEmojiStr(_emoji.code);
  if (emojiPixmap.isNull() || EmojiPickerSettings::snapshot().useSystemEmojiFont()) {
    setText(QString::fromStdString(_emoji.code));
    return;
  }

  emojiPixmap = emojiPixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  emojiPixmap.setDevicePixelRatio(_devicePixelRatio);

  setPixmap(emojiPixmap);
}

bool EmojiLabel::highlighted() {
  if (_devicePixelRatio != 1) {
    return styleSheet().isNull();
  }

  return _shadowEffect->isEnabled();
}
void EmojiLabel::setHighlighted(bool highlighted) {
  // QGraphicsDropShadowEffect breaks QPixmap::setDevicePixelRatio
  // https://bugreports.qt.io/browse/QTBUG-65035
  if (_devicePixelRatio != 1) {
    if (highlighted) {
      QColor emojiLabelHoverBgColor = palette().text().color();
      emojiLabelHoverBgColor.setAlphaF(0.33);
      setStyleSheet(QString("background-color: #%1").arg(emojiLabelHoverBgColor.rgba(), 0, 16));
    } else {
      setStyleSheet(QString());
    }

    return;
  }

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
