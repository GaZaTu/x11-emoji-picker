#include "EmojiLabel.hpp"
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <sstream>
#include "utf8.h"

EmojiLabel::EmojiLabel(QWidget* parent, const EmojiPickerSettings& settings) : QLabel(parent), _settings(settings) {
  setProperty("class", "EmojiLabel");
  setGraphicsEffect(_shadowEffect);
  setMouseTracking(true);

  _shadowColor = QColor(240, 240, 240);
  if (EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    _shadowColor = palette().text().color();
  }

  _shadowEffect->setColor(_shadowColor);
  _shadowEffect->setOffset(0);
  _shadowEffect->setBlurRadius(20);
  _shadowEffect->setEnabled(false);
}

EmojiLabel::EmojiLabel(QWidget* parent, const EmojiPickerSettings& settings, const Emoji& emoji) : EmojiLabel(parent, settings) {
  setEmoji(emoji);
}

void getCodepointsByEmojiStr(const std::string& emojiStr, const std::string& separator, std::stringstream& emojiHexCodeStream) {
  const utf8_int8_t* ptr = emojiStr.data();
  utf8_int32_t codepoint = 0;

  while (true) {
    bool first = codepoint == 0;

    ptr = utf8codepoint(ptr, &codepoint);
    if (codepoint == 0) {
      break;
    }

    if (!first) {
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

int defaultEmojiWidth = 0;
int invalidEmojiWidth = 0;

bool fontSupportsEmoji(const QFontMetrics& metrics, int textWidth) {
  if (invalidEmojiWidth == 0) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    invalidEmojiWidth = metrics.horizontalAdvance(u8"\U0001FFFD");
#else
    invalidEmojiWidth = metrics.width(u8"\U0001FFFD");
#endif
  }

  return textWidth != invalidEmojiWidth;
}

bool fontSupportsEmoji(const QFontMetrics& metrics, const QString& text) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  int textWidth = metrics.horizontalAdvance(text);
#else
  int textWidth = metrics.width(text);
#endif

  return fontSupportsEmoji(metrics, textWidth);
}

const Emoji& EmojiLabel::emoji() const {
  return _emoji;
}

void EmojiLabel::setEmoji(const Emoji& emoji, int w, int h) {
  _emoji = emoji;

  double pixelRatio = std::max(devicePixelRatioF(), 1.0);

  w = w * pixelRatio;
  h = h * pixelRatio;

  setAccessibleName(QString::fromStdString(_emoji.name));

  QPixmap emojiPixmap = getPixmapByEmojiStr(_emoji.code);
  _hasRealEmoji = !emojiPixmap.isNull();

  if (_hasRealEmoji && !_settings.useSystemEmojiFont()) {
    emojiPixmap = emojiPixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    emojiPixmap.setDevicePixelRatio(pixelRatio);

    setPixmap(emojiPixmap);
  } else if (_hasRealEmoji) {
    QString text = QString::fromStdString(_emoji.code);

    QFont textFont = font();
    textFont.setPixelSize(w);

    if (_settings.useSystemEmojiFontWidthHeuristics()) {
      if (defaultEmojiWidth == 0) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        defaultEmojiWidth = fontMetrics().horizontalAdvance(u8"\U0001F600");
#else
        defaultEmojiWidth = fontMetrics().width(u8"\U0001F600");
#endif
      }

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
      int textWidth = fontMetrics().horizontalAdvance(text);
#else
      int textWidth = fontMetrics().width(text);
#endif
      if (textWidth > defaultEmojiWidth) {
        double multiplier = (double)defaultEmojiWidth / (double)textWidth;
        textFont.setPixelSize((double)textFont.pixelSize() * multiplier);
      }
    }

    textFont.setPixelSize((double)textFont.pixelSize() / pixelRatio);

    setFont(textFont);
    setText(text);
    setMaximumSize(w * 1.10, h * 1.10);
  } else {
    QString text = QString::fromStdString(_emoji.code);

    QFont textFont = font();
    textFont.setPixelSize((double)w * 0.45 / pixelRatio);

    setFont(textFont);
    setText(text);
    setMaximumSize(w * 2.45 / pixelRatio, h * 1.10);
  }
}

bool EmojiLabel::highlighted() const {
  return _highlighted;
}

void EmojiLabel::setHighlighted(bool highlighted) {
  _highlighted = highlighted;

  _shadowEffect->setEnabled(_highlighted);

  if (!_hasRealEmoji) {
    if (_highlighted) {
      QColor emojiLabelHoverBgColor = _shadowColor;
      emojiLabelHoverBgColor.setAlphaF(0.1);
      setStyleSheet(QString("background-color: #%1").arg(emojiLabelHoverBgColor.rgba(), 0, 16));
    } else {
      setStyleSheet(QString());
    }
  }
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

bool EmojiLabel::hasRealEmoji() const {
  return _hasRealEmoji;
}
