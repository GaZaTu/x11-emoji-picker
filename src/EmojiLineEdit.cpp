#include "EmojiLineEdit.hpp"

EmojiLineEdit::EmojiLineEdit(QWidget* parent) : QLineEdit(parent) {
}

QWidget* EmojiLineEdit::containerWidget() {
  if (_container == nullptr) {
    _containerLayout = new QStackedLayout();
    _containerLayout->setStackingMode(QStackedLayout::StackAll);

    _container = new QWidget();
    _container->setLayout(_containerLayout);
    _container->setMaximumHeight(sizeHint().height());

    _previewLabel = new QLabel();
    _previewLabel->setText("");
    _previewLabel->setStyleSheet("padding-left: 5px; color: rgba(255, 255, 255, 50%);");

    _favsLabel = new EmojiLabel();
    _favsLabel->setEmoji({"", u8"⭐"});
    _favsLabel->setStyleSheet("margin-left: 285px;");

    _helpLabel = new EmojiLabel();
    _helpLabel->setEmoji({"", u8"🗃"});
    _helpLabel->setStyleSheet("margin-left: 315px;");

    _containerLayout->addWidget(_helpLabel);
    _containerLayout->addWidget(_favsLabel);
    _containerLayout->addWidget(_previewLabel);
    _containerLayout->addWidget(this);
  }

  return _container;
}
QLabel* EmojiLineEdit::previewLabel() {
  return _previewLabel;
}
EmojiLabel* EmojiLineEdit::favsLabel() {
  return _favsLabel;
}
EmojiLabel* EmojiLineEdit::helpLabel() {
  return _helpLabel;
}

std::string EmojiLineEdit::previewText() {
  return _previewLabel->text().toStdString();
}
void EmojiLineEdit::setPreviewText(const std::string& previewText) {
  QString previewTextAsQString = QString::fromStdString(previewText);
  previewTextAsQString.replace(0, text().length(), text());
  if (previewTextAsQString.length() > 30) {
    previewTextAsQString = previewTextAsQString.left(30) + "...";
  }

  _previewLabel->setText(previewTextAsQString);
}

void EmojiLineEdit::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
  case Qt::Key_Escape:
    emit escapePressed();
    break;
  case Qt::Key_Up:
  case Qt::Key_Down:
  case Qt::Key_Left:
  case Qt::Key_Right:
    emit arrowKeyPressed(event->key());
    break;
  case Qt::Key_F1:
  case Qt::Key_F2:
    emit functionKeyPressed(event->key());
    break;
  default:
    QLineEdit::keyPressEvent(event);
  }
}
