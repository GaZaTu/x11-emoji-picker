#include "EmojiLineEdit.hpp"

EmojiLineEdit::EmojiLineEdit(QWidget* parent) : QLineEdit(parent) {
  setStyleSheet("padding-left: 5px; padding-right: 5px;");
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

#ifdef __linux__
    QColor previewLabelTextColor = _previewLabel->palette().text().color();
    previewLabelTextColor.setAlphaF(0.6);
    _previewLabel->setStyleSheet(QString("padding-left: 5px; color: #%1;").arg(previewLabelTextColor.rgba(), 0, 16));
#elif _WIN32
    _previewLabel->setStyleSheet(QString("padding-left: 5px; color: rgba(240, 240, 240, 0.60); font-size: 16px;"));
#endif

    _iconsLayout = new QGridLayout();
    _iconsLayout->setAlignment(Qt::AlignRight);

    _iconsLayoutWidget = new QWidget();
    _iconsLayoutWidget->setLayout(_iconsLayout);

#ifdef __linux__
    QColor emojiLabelHoverBgColor = _iconsLayoutWidget->palette().text().color();
    emojiLabelHoverBgColor.setAlphaF(0.33);
    _iconsLayoutWidget->setStyleSheet(QString("EmojiLabel { padding: 1px; border-radius: 5px; } EmojiLabel:hover { "
                                              "background-color: #%1; }")
                                          .arg(emojiLabelHoverBgColor.rgba(), 0, 16));
#elif _WIN32
    _iconsLayoutWidget->setStyleSheet(QString("EmojiLabel { padding: 1px; border-radius: 5px; } EmojiLabel:hover { "
                                              "background-color: rgba(240, 240, 240, 0.33); }"));
#endif

    _favsLabel = new EmojiLabel();
    _favsLabel->setEmoji({"", u8"⭐"}, 20, 20);

    _helpLabel = new EmojiLabel();
    _helpLabel->setEmoji({"", u8"🗃"}, 20, 20);

    _iconsLayout->addWidget(_favsLabel, 0, 0);
    _iconsLayout->addWidget(_helpLabel, 0, 1);

    _containerLayout->addWidget(_iconsLayoutWidget);
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
  if (previewTextAsQString.length() > 36) {
    previewTextAsQString = previewTextAsQString.left(36) + "...";
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
  case Qt::Key_Tab:
    emit tabPressed();
    break;
  default:
    QLineEdit::keyPressEvent(event);
  }
}
