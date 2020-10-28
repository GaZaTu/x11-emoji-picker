#include "EmojiPicker.hpp"
#include "emojis.hpp"
#include <QSettings>
#include <functional>

template <typename T>
std::vector<T> readQSettingsArrayToStdVector(
    QSettings& settings, const QString& prefix, std::function<T(QSettings&)> readValue) {
  std::vector<T> data;

  const int size = settings.beginReadArray(prefix);
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);

    data.push_back(readValue(settings));
  }
  settings.endArray();

  return data;
}

template <typename T>
void writeQSettingsArrayFromStdVector(QSettings& settings, const QString& prefix, const std::vector<T>& data,
    std::function<void(QSettings&, const T&)> writeValue) {
  const int size = data.size();
  settings.beginWriteArray(prefix, size);
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);

    writeValue(settings, data[i]);
  }
  settings.endArray();
}

std::vector<std::string> readRecentEmojis() {
  QSettings settings;

  return readQSettingsArrayToStdVector<std::string>(settings, "recentEmojis", [](QSettings& settings) {
    return settings.value("emojiStr").toString().toStdString();
  });
}

void writeRecentEmojis(const std::vector<std::string>& recentEmojis) {
  QSettings settings;

  writeQSettingsArrayFromStdVector<std::string>(
      settings, "recentEmojis", recentEmojis, [](QSettings& settings, const std::string& emojiStr) {
        settings.setValue("emojiStr", QString::fromStdString(emojiStr));
      });
}

EmojiPicker::EmojiPicker(QWidget* parent) : QWidget(parent) {
  setLayout(&_mainLayout);

  _recentEmojis = readRecentEmojis();

  _emojiLayout.setAlignment(Qt::AlignTop | Qt::AlignLeft);

  _emojiLayoutWidget.setLayout(&_emojiLayout);
  _emojiLayoutWidget.setStyleSheet("EmojiLabel { padding: 2px; }");

  _mainLayout.addWidget(&_emojiEdit);
  _mainLayout.addWidget(&_emojiLayoutWidget);

  onTextChanged("");

  QObject::connect(&_emojiEdit, &EmojiLineEdit::textChanged, this, &EmojiPicker::onTextChanged);
  QObject::connect(&_emojiEdit, &EmojiLineEdit::returnPressed, this, &EmojiPicker::onReturnPressed);
  QObject::connect(&_emojiEdit, &EmojiLineEdit::arrowKeyPressed, this, &EmojiPicker::onArrowKeyPressed);
  QObject::connect(&_emojiEdit, &EmojiLineEdit::escapePressed, this, &EmojiPicker::onEscapePressed);
}

bool EmojiPicker::addEmojiLabel(EmojiLabel* emojiLabel, int& row, int& col) {
  if (col == 0 && row == 0) {
    _selectedEmojiLabel = emojiLabel;
    _selectedEmojiLabel->setHighlighted(true);
  }

  emojiLabel->setProperty("row", row);
  emojiLabel->setProperty("col", col);

  _emojiLayout.addWidget(emojiLabel, row, col);

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
  if (_selectedEmojiLabel != nullptr) {
    _selectedEmojiLabel->setHighlighted(false);
    _selectedEmojiLabel = nullptr;
  }

  while (auto* w = _emojiLayoutWidget.findChild<EmojiLabel*>()) {
    delete w;
  }

  int col = 0;
  int row = 0;

  std::string text = textQStr.toStdString();

  if (text == "") {
    for (const auto& emojiStr : _recentEmojis) {
      if (addEmojiLabel(new EmojiLabel(nullptr, emojiStr), row, col)) {
        break;
      }
    }

    return;
  }

  for (const auto& emoji : emojis) {
    const std::string& emojiKey = emoji.first;
    const std::string& emojiStr = emoji.second;

    auto it = std::search(emojiKey.begin(), emojiKey.end(), text.begin(), text.end(), [](char c1, char c2) {
      return std::tolower(c1) == std::tolower(c2);
    });

    if (it != emojiKey.begin()) {
      continue;
    }

    if (addEmojiLabel(new EmojiLabel(nullptr, emojiStr), row, col)) {
      break;
    }
  }
}

void EmojiPicker::onReturnPressed() {
  if (_selectedEmojiLabel == nullptr) {
    return;
  }

  const std::string& emojiStr = _selectedEmojiLabel->emojiStr();

  for (auto it = _recentEmojis.begin(); it != _recentEmojis.end();) {
    if (*it == emojiStr) {
      it = _recentEmojis.erase(it);
      break;
    } else {
      ++it;
    }
  }

  _recentEmojis.insert(_recentEmojis.begin(), emojiStr);

  if (_recentEmojis.size() > 40) {
    _recentEmojis.resize(40);
  }

  emit returnPressed(emojiStr);
}

void EmojiPicker::onArrowKeyPressed(int key) {
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

  for (EmojiLabel* emojiLabel : _emojiLayoutWidget.findChildren<EmojiLabel*>()) {
    int row = emojiLabel->property("row").toInt();
    int col = emojiLabel->property("col").toInt();

    if (row == rowToSelect && col == colToSelect) {
      _selectedEmojiLabel->setHighlighted(false);
      _selectedEmojiLabel = emojiLabel;
      _selectedEmojiLabel->setHighlighted(true);

      break;
    }
  }
}

void EmojiPicker::onEscapePressed() {
  writeRecentEmojis(_recentEmojis);

  emit escapePressed();
}
