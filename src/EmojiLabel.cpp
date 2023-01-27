#include "EmojiLabel.hpp"
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <sstream>
#include <unicode/schriter.h>
#include <unicode/unistr.h>

EmojiLabel::EmojiLabel(QWidget* parent, const EmojiPickerSettings& settings) : QLabel(parent), _settings(settings) {
  setProperty("class", "EmojiLabel");
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

EmojiLabel::EmojiLabel(QWidget* parent, const EmojiPickerSettings& settings, const Emoji& emoji) : EmojiLabel(parent, settings) {
  setEmoji(emoji);
}

void getCodepointsByEmojiStr(const std::string& emojiStr, const std::string& separator, std::stringstream& emojiHexCodeStream) {
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

  w = w * _devicePixelRatio;
  h = h * _devicePixelRatio;

  setAccessibleName(QString::fromStdString(_emoji.name));

  QPixmap emojiPixmap = getPixmapByEmojiStr(_emoji.code);
  _hasRealEmoji = !emojiPixmap.isNull();

  if (_hasRealEmoji && !_settings.useSystemEmojiFont()) {
    emojiPixmap = emojiPixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    emojiPixmap.setDevicePixelRatio(_devicePixelRatio);

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

    setFont(textFont);
    setText(text);
    setMaximumSize(w * 1.10, h * 1.10);
  } else {
    QString text = QString::fromStdString(_emoji.code);

    QFont textFont = font();
    textFont.setPixelSize(w * 0.45);

    setFont(textFont);
    setText(text);
    setMaximumSize(w * 2.45, h * 1.10);
  }
}

bool EmojiLabel::highlighted() const {
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

bool EmojiLabel::hasRealEmoji() const {
  return _hasRealEmoji;
}
