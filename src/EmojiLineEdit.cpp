#include "EmojiLineEdit.hpp"
#include "EmojiPickerSettings.hpp"

EmojiAction getEmojiActionForQKeyEvent(const QKeyEvent* event) {
  if (event->type() == QKeyEvent::KeyRelease) {
    return EmojiAction::INVALID;
  }

  if (event->modifiers() & Qt::MetaModifier) {
    return EmojiAction::INVALID;
  }

  if (event->modifiers() & Qt::ControlModifier) {
    if (event->text().toLower() == "c") {
      return EmojiAction::COPY_SELECTED_EMOJI;
    }

    switch (event->key()) {
    case Qt::Key_Up:
      return EmojiAction::PAGE_UP;

    case Qt::Key_Down:
      return EmojiAction::PAGE_DOWN;
    }

    return EmojiAction::INVALID;
  }

  switch (event->key()) {
  case Qt::Key_Escape:
    return EmojiAction::DISABLE;

  case Qt::Key_Return:
    return EmojiAction::COMMIT_EMOJI;

  case Qt::Key_Tab:
    return EmojiAction::SWITCH_VIEW_MODE;

  case Qt::Key_Backtab:
    return EmojiAction::SWITCH_VIEW_MODE;

  case Qt::Key_Up:
    return EmojiAction::UP;

  case Qt::Key_Down:
    return EmojiAction::DOWN;

  case Qt::Key_Left:
    return EmojiAction::LEFT;

  case Qt::Key_Right:
    return EmojiAction::RIGHT;

  case Qt::Key_PageUp:
    return EmojiAction::PAGE_UP;

  case Qt::Key_PageDown:
    return EmojiAction::PAGE_DOWN;

  case Qt::Key_F4:
    return EmojiAction::OPEN_SETTINGS;

  case Qt::Key_F5:
    return EmojiAction::TOGGLE_INPUT_MODE;
  }

  return EmojiAction::INVALID;
}

EmojiLineEdit::EmojiLineEdit(QWidget* parent) : QLineEdit(parent) {
  setProperty("class", "EmojiLineEdit");
}

void EmojiLineEdit::keyPressEvent(QKeyEvent* event) {
  EmojiAction action = getEmojiActionForQKeyEvent(event);

  if (action == EmojiAction::INVALID) {
    QString oldText = text();
    QLineEdit::keyPressEvent(event);
    QString newText = text();

    if (newText != oldText) {
      emit updateEmojiList();
    }
  } else {
    emit processKeyEvent(event);
  }
}

void EmojiLineEdit::focusOutEvent(QFocusEvent* event) {
  event->accept();

  if (EmojiPickerSettings::snapshot().closeOnFocusLost()) {
    if (event->lostFocus()) {
      emit disable();
    }
  }
}
