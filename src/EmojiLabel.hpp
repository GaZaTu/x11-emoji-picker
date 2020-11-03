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
  void setEmoji(const Emoji& emoji);

  bool highlighted();
  void setHighlighted(bool highlighted);

signals:
  void mousePressed(QMouseEvent* ev);

protected:
  void mousePressEvent(QMouseEvent* ev) override;

private:
  Emoji _emoji;

  QGraphicsDropShadowEffect* _shadowEffect = new QGraphicsDropShadowEffect();
};
