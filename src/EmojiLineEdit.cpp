#include "EmojiLineEdit.hpp"
#include "EmojiPickerSettings.hpp"

EmojiLineEdit::EmojiLineEdit(QWidget* parent) : QLineEdit(parent) {
  setProperty("class", "EmojiLineEdit");
}

QWidget* EmojiLineEdit::containerWidget() {
  if (_container == nullptr) {
    _containerLayout = new QStackedLayout();
    _containerLayout->setStackingMode(QStackedLayout::StackAll);

    _container = new QWidget();
    _container->setLayout(_containerLayout);
    _container->setMaximumHeight(sizeHint().height());

    _previewLabel = new QLabel();
    _previewLabel->setProperty("class", "EmojiLineEdit_previewLabel");
    _previewLabel->setText("");

    if (EmojiPickerSettings::snapshot().useSystemQtTheme()) {
      setTextMargins(1, 0, 0, 0);

      _previewLabel->setIndent(fontMetrics().averageCharWidth());

      QColor previewLabelTextColor = _previewLabel->palette().text().color();
      previewLabelTextColor.setAlphaF(0.6);
      _previewLabel->setStyleSheet(QString("color: #%1;").arg(previewLabelTextColor.rgba(), 0, 16));
    }

    _iconsLayout = new QGridLayout();
    _iconsLayout->setAlignment(Qt::AlignRight);

    _iconsLayoutWidget = new QWidget();
    _iconsLayoutWidget->setLayout(_iconsLayout);

    if (EmojiPickerSettings::snapshot().useSystemQtTheme()) {
      QColor emojiLabelHoverBgColor = _iconsLayoutWidget->palette().text().color();
      emojiLabelHoverBgColor.setAlphaF(0.33);
      _iconsLayoutWidget->setStyleSheet(
          QString("EmojiLabel { padding: 1px 2px 1px 2px; border-radius: 5px; } EmojiLabel:hover { "
                  "background-color: #%1; }")
              .arg(emojiLabelHoverBgColor.rgba(), 0, 16));
    }

    _favsLabel = new EmojiLabel();
    _favsLabel->setProperty("class", "EmojiLineEdit_favsLabel");
    _favsLabel->setEmoji({"", u8"⭐"}, 16, 16);

    _helpLabel = new EmojiLabel();
    _helpLabel->setProperty("class", "EmojiLineEdit_helpLabel");
    _helpLabel->setEmoji({"", u8"🗃"}, 16, 16);

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

  int indexOfText = std::max(previewTextAsQString.indexOf(text(), 0, Qt::CaseInsensitive), 0);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  int textWidth = fontMetrics().horizontalAdvance(previewTextAsQString.left(indexOfText));
#else
  int textWidth = fontMetrics().width(previewTextAsQString.left(indexOfText));
#endif

  previewTextAsQString.replace(indexOfText, text().length(), text());
  if (previewTextAsQString.length() > 36) {
    previewTextAsQString = previewTextAsQString.left(36) + "...";
  }

  int defaultMarginLeft = 0;
  if (EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    defaultMarginLeft = 1;
  }

  setTextMargins(textWidth + defaultMarginLeft, 0, 0, 0);

  _previewLabel->setText(previewTextAsQString);
}

void EmojiLineEdit::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
  case Qt::Key_Return:
    emit returnPressed(*event);
    break;
  case Qt::Key_Escape:
    emit escapePressed(*event);
    break;
  case Qt::Key_Up:
  case Qt::Key_Down:
  case Qt::Key_Left:
  case Qt::Key_Right:
    emit arrowKeyPressed(*event);
    break;
  case Qt::Key_F1:
  case Qt::Key_F2:
  case Qt::Key_F3:
  case Qt::Key_F4:
  case Qt::Key_F5:
  case Qt::Key_F6:
    emit functionKeyPressed(*event);
    break;
  case Qt::Key_Tab:
    emit tabPressed(*event);
    break;
  case Qt::Key_C:
    if ((event->modifiers() & Qt::ControlModifier) != 0 && !hasSelectedText()) {
      emit copyPressed(*event);
      break;
    }
  default:
    QLineEdit::keyPressEvent(event);
  }
}
