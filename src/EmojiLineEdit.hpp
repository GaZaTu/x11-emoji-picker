#pragma once

#include "EmojiLabel.hpp"
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QStackedLayout>

enum class EmojiAction {
  INVALID,
  COPY_SELECTED_EMOJI,
  DISABLE,
  COMMIT_EMOJI,
  SWITCH_VIEW_MODE,
  UP,
  DOWN,
  LEFT,
  RIGHT,
  PAGE_UP,
  PAGE_DOWN,
  OPEN_SETTINGS,
  TOGGLE_INPUT_MODE,
};

EmojiAction getEmojiActionForQKeyEvent(const QKeyEvent* event);

class EmojiLineEdit : public QLineEdit {
  Q_OBJECT

public:
  explicit EmojiLineEdit(QWidget* parent);

signals:
  void updateEmojiList();
  void processKeyEvent(const QKeyEvent* event);
  void disable();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
};
