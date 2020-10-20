#pragma once

#include <QLabel>

class EmojiLabel : public QLabel {
  Q_OBJECT

public:
  explicit EmojiLabel(QWidget* parent = nullptr);
  explicit EmojiLabel(QWidget* parent, const std::string& emojiStr);

  const std::string& emojiStr();
  void setEmojiStr(const std::string& emojiStr);

private:
  std::string _emojiStr;
};
