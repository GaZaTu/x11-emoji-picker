#pragma once

#include <QLabel>
#include <QGraphicsDropShadowEffect>

class EmojiLabel : public QLabel {
  Q_OBJECT

public:
  explicit EmojiLabel(QWidget* parent = nullptr);
  explicit EmojiLabel(QWidget* parent, const std::string& emojiStr);

  const std::string& emojiStr();
  void setEmojiStr(const std::string& emojiStr);

  bool highlighted();
  void setHighlighted(bool highlighted);

private:
  std::string _emojiStr;

  QGraphicsDropShadowEffect _shadowEffect;
};
