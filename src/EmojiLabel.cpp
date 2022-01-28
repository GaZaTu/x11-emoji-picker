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

int calculateTextWidth(const QFontMetrics& metrics, const QString& text) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  return metrics.horizontalAdvance(text);
#else
  return metrics.width(text);
#endif
}

int defaultEmojiWidth = 0;
int invalidEmojiWidth = 0;

bool fontSupportsEmoji(const QFontMetrics& metrics, int textWidth) {
  if (invalidEmojiWidth == 0) {
    invalidEmojiWidth = calculateTextWidth(metrics, u8"\U0001FFFD");
  }

  return textWidth != invalidEmojiWidth;
}

bool fontSupportsEmoji(const QFontMetrics& metrics, const QString& text) {
  int textWidth = calculateTextWidth(metrics, text);

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

  if (EmojiPickerSettings::snapshot().useSystemEmojiFont()) {
    QString text = QString::fromStdString(_emoji.code);

    QFont textFont = font();
    textFont.setPixelSize(28);

    if (EmojiPickerSettings::snapshot().useSystemEmojiFontWidthHeuristics()) {
      if (defaultEmojiWidth == 0) {
        defaultEmojiWidth = calculateTextWidth(fontMetrics(), u8"\U0001F600");
      }

      int textWidth = calculateTextWidth(fontMetrics(), text);
      if (textWidth > defaultEmojiWidth) {
        double multiplier = (double)defaultEmojiWidth / (double)textWidth;
        textFont.setPixelSize((double)textFont.pixelSize() * multiplier);
      }
    }

    setFont(textFont);
    setText(text);
  } else {
    QPixmap emojiPixmap = getPixmapByEmojiStr(_emoji.code);
    if (emojiPixmap.isNull()) {
      setText(QString::fromStdString(_emoji.code));
      return;
    }

    emojiPixmap = emojiPixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    emojiPixmap.setDevicePixelRatio(_devicePixelRatio);

    setPixmap(emojiPixmap);
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
