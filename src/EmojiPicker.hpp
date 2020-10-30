#pragma once

#include "EmojiLabel.hpp"
#include "EmojiLineEdit.hpp"
#include "emojis.hpp"
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

  std::vector<Emoji> _recentEmojis;

  EmojiLabel* _selectedEmojiLabel = nullptr;

  QVBoxLayout* _mainLayout = new QVBoxLayout();

  QWidget* _emojiLayoutWidget = new QWidget();
  QGridLayout* _emojiLayout = new QGridLayout();

  EmojiLineEdit* _emojiEdit = new EmojiLineEdit();

  void setSelectedEmojiLabel(EmojiLabel* emojiLabel);

  bool addEmojiLabel(EmojiLabel* emojiLabel, int& row, int& col);

private slots:
  void onTextChanged(const QString& text);
  void onReturnPressed();
  void onArrowKeyPressed(int key);
  void onEscapePressed();
};
