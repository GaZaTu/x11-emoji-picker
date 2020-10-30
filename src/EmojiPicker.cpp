#include "EmojiPicker.hpp"
#include "EmojiPickerSettings.hpp"
#include <QCompleter>
#include <QStackedLayout>

EmojiPicker::EmojiPicker(QWidget* parent) : QWidget(parent) {
  setLayout(_mainLayout);

  _recentEmojis = EmojiPickerSettings().recentEmojis();

  _emojiLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  _emojiLayoutWidget->setLayout(_emojiLayout);
  _emojiLayoutWidget->setStyleSheet("EmojiLabel { padding: 2px; }");

  _mainLayout->addWidget(_emojiEdit->containerWidget());
  _mainLayout->addWidget(_emojiLayoutWidget);

  onTextChanged("");

  QObject::connect(_emojiEdit, &EmojiLineEdit::textChanged, this, &EmojiPicker::onTextChanged);
  QObject::connect(_emojiEdit, &EmojiLineEdit::returnPressed, this, &EmojiPicker::onReturnPressed);
  QObject::connect(_emojiEdit, &EmojiLineEdit::arrowKeyPressed, this, &EmojiPicker::onArrowKeyPressed);
  QObject::connect(_emojiEdit, &EmojiLineEdit::escapePressed, this, &EmojiPicker::onEscapePressed);
}

void EmojiPicker::setSelectedEmojiLabel(EmojiLabel* emojiLabel) {
  if (_selectedEmojiLabel != nullptr) {
    _selectedEmojiLabel->setHighlighted(false);
  }

  _selectedEmojiLabel = emojiLabel;
  _selectedEmojiLabel->setHighlighted(true);

  const std::string localeKey = EmojiPickerSettings().localeKey();
  const std::string& emojiKey = emojiLabel->emoji().nameByLocale(localeKey);

  _emojiEdit->setPreviewText(emojiKey);
}

bool EmojiPicker::addEmojiLabel(EmojiLabel* emojiLabel, int& row, int& col) {
  if (col == 0 && row == 0) {
    setSelectedEmojiLabel(emojiLabel);
  }

  emojiLabel->setProperty("row", row);
  emojiLabel->setProperty("col", col);

  _emojiLayout->addWidget(emojiLabel, row, col);

  col += 1;

  if (col == cols) {
    col = 0;
    row += 1;
  }

  if (row == rows) {
    return true;
  }

  return false;
}

void EmojiPicker::onTextChanged(const QString& textQStr) {
  _emojiEdit->setPreviewText("");

  if (_selectedEmojiLabel != nullptr) {
    _selectedEmojiLabel->setHighlighted(false);
    _selectedEmojiLabel = nullptr;
  }

  while (auto* w = _emojiLayoutWidget->findChild<EmojiLabel*>()) {
    delete w;
  }

  int col = 0;
  int row = 0;

  const std::string text = textQStr.toStdString();

  if (text == "") {
    for (const auto& emoji : _recentEmojis) {
      if (addEmojiLabel(new EmojiLabel(nullptr, emoji), row, col)) {
        break;
      }
    }

    return;
  }

  const std::string localeKey = EmojiPickerSettings().localeKey();

  for (const auto& emoji : emojis) {
    const std::string& emojiKey = emoji.nameByLocale(localeKey);

    auto it = std::search(emojiKey.begin(), emojiKey.end(), text.begin(), text.end(), [](char c1, char c2) {
      return std::tolower(c1) == std::tolower(c2);
    });

    if (it != emojiKey.begin()) {
      continue;
    }

    if (addEmojiLabel(new EmojiLabel(nullptr, emoji), row, col)) {
      break;
    }
  }
}

void EmojiPicker::onReturnPressed() {
  if (_selectedEmojiLabel == nullptr) {
    return;
  }

  const Emoji& emoji = _selectedEmojiLabel->emoji();

  for (auto it = _recentEmojis.begin(); it != _recentEmojis.end();) {
    if (*it == emoji) {
      it = _recentEmojis.erase(it);
      break;
    } else {
      ++it;
    }
  }

  _recentEmojis.insert(_recentEmojis.begin(), emoji);

  if (_recentEmojis.size() > 40) {
    _recentEmojis.resize(40);
  }

  emit returnPressed(emoji.code);
}

void EmojiPicker::onArrowKeyPressed(int key) {
  if (_selectedEmojiLabel == nullptr) {
    return;
  }

  int rowToSelect = _selectedEmojiLabel->property("row").toInt();
  int colToSelect = _selectedEmojiLabel->property("col").toInt();

  switch (key) {
  case Qt::Key_Up:
    rowToSelect -= 1;
    break;
  case Qt::Key_Down:
    rowToSelect += 1;
    break;
  case Qt::Key_Left:
    colToSelect -= 1;
    break;
  case Qt::Key_Right:
    colToSelect += 1;
    break;
  }

  for (EmojiLabel* emojiLabel : _emojiLayoutWidget->findChildren<EmojiLabel*>()) {
    int row = emojiLabel->property("row").toInt();
    int col = emojiLabel->property("col").toInt();

    if (row == rowToSelect && col == colToSelect) {
      setSelectedEmojiLabel(emojiLabel);

      break;
    }
  }
}

void EmojiPicker::onEscapePressed() {
  EmojiPickerSettings().setRecentEmojis(_recentEmojis);

  emit escapePressed();
}
