#include "EmojiLineEdit.hpp"

EmojiLineEdit::EmojiLineEdit(QWidget* parent) : QLineEdit(parent) {
}

QWidget* EmojiLineEdit::containerWidget() {
  _containerLayout->setStackingMode(QStackedLayout::StackAll);
  _container->setLayout(_containerLayout);
  _container->setMaximumHeight(sizeHint().height());

  _previewLabel->setText("");
  _previewLabel->setStyleSheet("padding-left: 5px; color: rgba(255, 255, 255, 50%);");

  _containerLayout->addWidget(_previewLabel);
  _containerLayout->addWidget(this);

  return _container;
}

std::string EmojiLineEdit::previewText() {
  return _previewLabel->text().toStdString();
}
void EmojiLineEdit::setPreviewText(const std::string& previewText) {
  QString previewTextAsQString = QString::fromStdString(previewText);
  previewTextAsQString.replace(0, text().length(), text());
  if (previewTextAsQString.length() > 45) {
    previewTextAsQString = previewTextAsQString.left(45) + "...";
  }

  _previewLabel->setText(previewTextAsQString);
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
