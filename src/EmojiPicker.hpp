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

Q_SIGNALS:
  void returnPressed(const std::string& emojiStr);
  void escapePressed();

private:
  static const int cols = 10;
  static const int rows = 4;

  EmojiLabel* _selectedEmojiLabel = nullptr;

  QVBoxLayout _mainLayout;

  QWidget _emojiLayoutWidget;
  QGridLayout _emojiLayout;

  EmojiLineEdit _emojiEdit;

  bool addEmojiLabel(EmojiLabel* emojiLabel, int& row, int& col);

  void onTextChanged(const QString& text);
  void onReturnPressed();
  void onArrowKeyPressed(int key);
  void onEscapePressed();
};
