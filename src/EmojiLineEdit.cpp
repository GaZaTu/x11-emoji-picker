#include "EmojiLineEdit.hpp"

EmojiLineEdit::EmojiLineEdit(QWidget* parent) : QLineEdit(parent) {
}

void EmojiLineEdit::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    emit escapePressed();
  } else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down || event->key() == Qt::Key_Left ||
      event->key() == Qt::Key_Right) {
    emit arrowKeyPressed(event->key());
  } else {
    // default handler for event
    QLineEdit::keyPressEvent(event);
  }
}
