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
  void returnPressed(const std::string& emojiStr, bool closeAfter);
  void escapePressed();
  void toggleInputMethod();

protected:
  void wheelEvent(QWheelEvent* event) override;

private:
  static const int cols = 10;
  static const int rows = 4;

  std::vector<Emoji> _recentEmojis;
  bool _skinTonesDisabled = false;
  bool _gendersDisabled = false;
  int _maxEmojiVersion = -1;
  std::vector<Emoji> _aliasedEmojis;
  std::string _settingsPath;

  EmojiLabel* _selectedEmojiLabel = nullptr;

  QVBoxLayout* _mainLayout = new QVBoxLayout();

  QWidget* _emojiLayoutWidget = new QWidget();
  QGridLayout* _emojiLayout = new QGridLayout();

  EmojiLineEdit* _emojiEdit = new EmojiLineEdit();

  int _helpEmojiListIdx = -1;
  int _helpEmojiListDir = +1;
  Emoji _helpEmojiListStartEmoji = {"", ""};

  void setSelectedEmojiLabel(EmojiLabel* emojiLabel);

  bool addEmojiLabel(EmojiLabel* emojiLabel, int& row, int& col);

  bool isDisabledEmoji(const Emoji& emoji);

  void clearView();
  void fillViewWithRecentEmojis();
  void fillViewWithEmojisByText(const std::string& text);
  void fillViewWithEmojisByList();

private:
  void onTextChanged(const QString& text);
  void onReturnPressed(const QKeyEvent& event);
  void onArrowKeyPressed(const QKeyEvent& event);
  void onEscapePressed(const QKeyEvent& event);
  void onFunctionKeyPressed(const QKeyEvent& event);
  void onTabPressed(const QKeyEvent& event);
  void onCopyPressed(const QKeyEvent& event);

  void onFavsPressed(QMouseEvent* ev);
  void onHelpPressed(QMouseEvent* ev);
};
