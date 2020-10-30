#pragma once

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPixmap>
#include "emojis.hpp"

class EmojiLabel : public QLabel {
  Q_OBJECT

public:
  explicit EmojiLabel(QWidget* parent = nullptr);
  explicit EmojiLabel(QWidget* parent, const Emoji& emoji);

  const Emoji& emoji();
  void setEmoji(const Emoji& emoji);

  bool highlighted();
  void setHighlighted(bool highlighted);

private:
  Emoji _emoji;

  QGraphicsDropShadowEffect* _shadowEffect = new QGraphicsDropShadowEffect();
};
