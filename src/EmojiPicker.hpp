#pragma once

#include "EmojiLabel.hpp"
#include "EmojiLineEdit.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>

class EmojiPicker : public QWidget {
  Q_OBJECT

public:
  explicit EmojiPicker(QWidget* parent = nullptr);

signals:
  void returnPressed(const std::string& emojiStr);
  void escapePressed();

private:
  static const int cols = 10;
  static const int rows = 4;

  std::vector<std::string> _recentEmojis;

  EmojiLabel* _selectedEmojiLabel = nullptr;

  QVBoxLayout _mainLayout;

  QWidget _emojiLayoutWidget;
  QGridLayout _emojiLayout;

  EmojiLineEdit _emojiEdit;

  bool addEmojiLabel(EmojiLabel* emojiLabel, int& row, int& col);

private slots:
  void onTextChanged(const QString& text);
  void onReturnPressed();
  void onArrowKeyPressed(int key);
  void onEscapePressed();
};
