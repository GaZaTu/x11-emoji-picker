#pragma once

#include "emojis.hpp"
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPixmap>

class EmojiLabel : public QLabel {
  Q_OBJECT

public:
  explicit EmojiLabel(QWidget* parent = nullptr);
  explicit EmojiLabel(QWidget* parent, const Emoji& emoji);

  const Emoji& emoji();
  void setEmoji(const Emoji& emoji, int w = 24, int h = 24);

  bool highlighted();
  void setHighlighted(bool highlighted);

signals:
  void mousePressed(QMouseEvent* ev);

protected:
  void mousePressEvent(QMouseEvent* ev) override;
  void mouseMoveEvent(QMouseEvent* ev) override;

private:
  Emoji _emoji;
  double _devicePixelRatio;

  QGraphicsDropShadowEffect* _shadowEffect = new QGraphicsDropShadowEffect();
};
