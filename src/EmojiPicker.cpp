#include "EmojiPicker.hpp"
#include "emojis.hpp"
#include <fstream>

std::vector<std::string> loadRecentEmojis() {
  std::vector<std::string> recentEmojis;
  std::ifstream file("./recentEmojis.txt");

  std::string line;
  while (std::getline(file, line)) {
    if (recentEmojis.size() == 40) {
      break;
    }

    if (line == "") {
      continue;
    }

    recentEmojis.push_back(line);
  }

  return recentEmojis;
}

void saveRecentEmojis(const std::vector<std::string>& recentEmojis) {
  if (recentEmojis.size() == 0) {
    return;
  }

  std::ofstream file("./recentEmojis.txt");

  for (const auto& line : recentEmojis) {
    file << line << std::endl;
  }
}

std::vector<std::string> recentEmojis = loadRecentEmojis();

const QString stylesheetEmojiLayout = "EmojiLabel { padding: 2px; border-radius: 6px; }";
const QString stylesheetSelectedEmojiLabel = "EmojiLabel { background-color: #bbbbbb; }";

EmojiPicker::EmojiPicker(QWidget* parent) : QWidget(parent) {
  setLayout(&_mainLayout);

  _emojiLayout.setAlignment(Qt::AlignTop | Qt::AlignLeft);

  _emojiLayoutWidget.setLayout(&_emojiLayout);
  _emojiLayoutWidget.setStyleSheet(stylesheetEmojiLayout);

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
    emojiLabel->setStyleSheet(stylesheetSelectedEmojiLabel);
    _selectedEmojiLabel = emojiLabel;
  }

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
  _selectedEmojiLabel = nullptr;

  while (auto* w = _emojiLayoutWidget.findChild<QLabel*>()) {
    delete w;
  }

  int col = 0;
  int row = 0;

  std::string text = textQStr.toStdString();

  if (text == "") {
    for (const auto& emojiStr : recentEmojis) {
      if (addEmojiLabel(new EmojiLabel(nullptr, emojiStr), row, col)) {
        break;
      }
    }

    return;
  }

  for (const auto& emoji : emojis) {
    const std::string& emojiKey = emoji.first;
    const std::string& emojiStr = emoji.second;

    auto it =
        std::search(emojiKey.begin(), emojiKey.end(), text.begin(), text.end(), [](char c1, char c2) {
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

  for (auto it = recentEmojis.begin(); it != recentEmojis.end();) {
    if (*it == emojiStr) {
      it = recentEmojis.erase(it);
      break;
    } else {
      ++it;
    }
  }

  recentEmojis.insert(recentEmojis.begin(), emojiStr);

  if (recentEmojis.size() > 40) {
    recentEmojis.resize(40);
  }

  emit returnPressed(emojiStr);
}

void EmojiPicker::onArrowKeyPressed(int key) {
  if (key == Qt::Key_Up) {
    EmojiLabel* prevEmojiLabel = nullptr;

    for (EmojiLabel* emojiLabel : _emojiLayoutWidget.findChildren<EmojiLabel*>()) {
      if (emojiLabel == _selectedEmojiLabel) {
        if (prevEmojiLabel != nullptr) {
          _selectedEmojiLabel->setStyleSheet("");
          _selectedEmojiLabel = prevEmojiLabel;
          _selectedEmojiLabel->setStyleSheet(stylesheetSelectedEmojiLabel);
        }

        break;
      }

      prevEmojiLabel = emojiLabel;
    }
  }

  if (key == Qt::Key_Down) {
    bool prevWasSelectedEmojiLabel = false;

    for (EmojiLabel* emojiLabel : _emojiLayoutWidget.findChildren<EmojiLabel*>()) {
      if (prevWasSelectedEmojiLabel) {
        _selectedEmojiLabel->setStyleSheet("");
        _selectedEmojiLabel = emojiLabel;
        _selectedEmojiLabel->setStyleSheet(stylesheetSelectedEmojiLabel);

        break;
      }

      if (emojiLabel == _selectedEmojiLabel) {
        prevWasSelectedEmojiLabel = true;
      }
    }
  }
}

void EmojiPicker::onEscapePressed() {
  saveRecentEmojis(recentEmojis);

  emit escapePressed();
}
