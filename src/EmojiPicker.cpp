#include "EmojiPicker.hpp"
#include "EmojiPickerSettings.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>

EmojiPicker::EmojiPicker(QWidget* parent) : QWidget(parent) {
  setLayout(_mainLayout);

  _recentEmojis = EmojiPickerSettings::snapshot().recentEmojis();
  _skinTonesDisabled = EmojiPickerSettings::snapshot().skinTonesDisabled();
  _gendersDisabled = EmojiPickerSettings::snapshot().gendersDisabled();
  _maxEmojiVersion = EmojiPickerSettings::snapshot().maxEmojiVersion();
  _aliasedEmojis = EmojiPickerSettings::snapshot().aliasedEmojis();
  _settingsPath = EmojiPickerSettings::snapshot().fileName().toStdString();

  _emojiLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  _emojiLayoutWidget->setLayout(_emojiLayout);

  if (EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    int paddingPx = 2;
    if (EmojiPickerSettings::snapshot().useSystemEmojiFont()) {
      paddingPx = 0;
    }

    QColor emojiLabelHoverBgColor = _emojiLayoutWidget->palette().text().color();
    emojiLabelHoverBgColor.setAlphaF(0.33);

    QString stylesheet = QString("EmojiLabel { padding: %1px; border-radius: 5px; } EmojiLabel:hover { "
                                 "background-color: #%2; }")
                             .arg(paddingPx)
                             .arg(emojiLabelHoverBgColor.rgba(), 0, 16);

    _emojiLayoutWidget->setStyleSheet(stylesheet);
  }

  _mainLayout->addWidget(_emojiEdit->containerWidget());
  _mainLayout->addWidget(_emojiLayoutWidget);

  _inputMethodLabel->setStyleSheet("font-size: 11px;");
  setInputMethod("unknown");

  if (!EmojiPickerSettings::snapshot().hideInputMethod()) {
    _mainLayout->addWidget(_inputMethodLabel, 0, Qt::AlignBottom);
  }

  fillViewWithRecentEmojis();

  QObject::connect(_emojiEdit, &EmojiLineEdit::textEdited, this, &EmojiPicker::onTextChanged);
  QObject::connect(_emojiEdit, &EmojiLineEdit::returnPressed, this, &EmojiPicker::onReturnPressed);
  QObject::connect(_emojiEdit, &EmojiLineEdit::arrowKeyPressed, this, &EmojiPicker::onArrowKeyPressed);
  QObject::connect(_emojiEdit, &EmojiLineEdit::escapePressed, this, &EmojiPicker::onEscapePressed);
  QObject::connect(_emojiEdit, &EmojiLineEdit::functionKeyPressed, this, &EmojiPicker::onFunctionKeyPressed);
  QObject::connect(_emojiEdit, &EmojiLineEdit::tabPressed, this, &EmojiPicker::onTabPressed);
  QObject::connect(_emojiEdit, &EmojiLineEdit::copyPressed, this, &EmojiPicker::onCopyPressed);

  QObject::connect(_emojiEdit->favsLabel(), &EmojiLabel::mousePressed, this, &EmojiPicker::onFavsPressed);
  QObject::connect(_emojiEdit->helpLabel(), &EmojiLabel::mousePressed, this, &EmojiPicker::onHelpPressed);
}

void EmojiPicker::setInputMethod(const std::string& InputMethod) {
  _inputMethodLabel->setText(QString("InputMethod: ") + QString::fromStdString(InputMethod));
}

void EmojiPicker::wheelEvent(QWheelEvent* event) {
  event->accept();

  QPoint stepsDelta;
  QPoint pixelDelta = event->pixelDelta();
  QPoint degreesDelta = event->angleDelta() / 8;

  if (!pixelDelta.isNull()) {
    stepsDelta = pixelDelta / 15;
  } else if (!degreesDelta.isNull()) {
    stepsDelta = degreesDelta / 15;
  }

  if (stepsDelta.isNull()) {
    return;
  }

  switch (stepsDelta.y()) {
  case +1:
    onArrowKeyPressed(QKeyEvent(QEvent::KeyPress, Qt::Key_Up, {}));
    break;
  case -1:
    onArrowKeyPressed(QKeyEvent(QEvent::KeyPress, Qt::Key_Down, {}));
    break;
  }
}

void EmojiPicker::setSelectedEmojiLabel(EmojiLabel* emojiLabel) {
  if (_selectedEmojiLabel != nullptr) {
    _selectedEmojiLabel->setHighlighted(false);
  }

  _selectedEmojiLabel = emojiLabel;
  _selectedEmojiLabel->setHighlighted(true);

  QVariant alias = emojiLabel->property("alias");
  QString previewText;

  if (alias.isNull()) {
    previewText = tr(emojiLabel->emoji().name.data());
  } else {
    previewText = alias.toString();
  }

  _emojiEdit->setPreviewText(previewText.toStdString());
}

bool EmojiPicker::addEmojiLabel(EmojiLabel* emojiLabel, int& row, int& col) {
  for (EmojiLabel* other : _emojiLayoutWidget->findChildren<EmojiLabel*>()) {
    if (other->emoji() == emojiLabel->emoji()) {
      return false;
    }
  }

  if (col == 0 && row == 0) {
    setSelectedEmojiLabel(emojiLabel);
  }

  QObject::connect(emojiLabel, &EmojiLabel::mousePressed, [this, emojiLabel]() {
    EmojiLabel* selectedEmojiLabel = _selectedEmojiLabel;

    _selectedEmojiLabel = emojiLabel;
    onReturnPressed(QKeyEvent(QEvent::KeyPress, Qt::Key_Return, {}));
    _selectedEmojiLabel = selectedEmojiLabel;
  });

  emojiLabel->setFixedWidth(280 / cols);
  emojiLabel->setFixedHeight(120 / rows);

  QString className = "EmojiPicker_emojiLabel";
  if (EmojiPickerSettings::snapshot().useSystemEmojiFont()) {
    className += " useFont";
  }
  emojiLabel->setProperty("class", className);

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

bool EmojiPicker::isDisabledEmoji(const Emoji& emoji) {
  if (_maxEmojiVersion != -1 && (emoji.version > _maxEmojiVersion)) {
    return true;
  }

  if (_skinTonesDisabled && emoji.isSkinToneVariation()) {
    return true;
  }

  if (_gendersDisabled && emoji.isGenderVariation()) {
    return true;
  }

  if (EmojiPickerSettings::snapshot().useSystemEmojiFont() && EmojiPickerSettings::snapshot().useSystemEmojiFontWidthHeuristics()) {
    if (!fontSupportsEmoji(fontMetrics(), QString::fromStdString(emoji.code))) {
      return true;
    }
  }

  return false;
}

void EmojiPicker::clearView() {
  _emojiEdit->setPreviewText("");

  if (_selectedEmojiLabel != nullptr) {
    _selectedEmojiLabel->setHighlighted(false);
    _selectedEmojiLabel = nullptr;
  }

  while (auto* w = _emojiLayoutWidget->findChild<EmojiLabel*>()) {
    delete w;
  }

  _emojiEdit->favsLabel()->setHighlighted(false);
  _emojiEdit->helpLabel()->setHighlighted(false);
}

void EmojiPicker::fillViewWithRecentEmojis() {
  _emojiEdit->favsLabel()->setHighlighted(true);

  int col = 0;
  int row = 0;

  for (const auto& emoji : _recentEmojis) {
    if (addEmojiLabel(new EmojiLabel(nullptr, emoji), row, col)) {
      break;
    }
  }
}

bool textMatchesEmojiName(const std::string& text, const std::string& emojiKey, bool mustStartWith) {
  auto found = std::search(emojiKey.begin(), emojiKey.end(), text.begin(), text.end(), [](char c1, char c2) {
    return std::tolower(c1) == std::tolower(c2);
  });

  if (EmojiPickerSettings::snapshot().enableEmojiIncludesSearch() && text.length() >= 3 && !mustStartWith) {
    return found != emojiKey.end();
  } else {
    return found == emojiKey.begin();
  }
}

void EmojiPicker::fillViewWithEmojisByText(const std::string& text) {
  int col = 0;
  int row = 0;

  _helpEmojiListIdx = -1;

  bool aliasExactMatching = EmojiPickerSettings::snapshot().aliasExactMatching();

  for (const auto& alias : _aliasedEmojis) {
    if (aliasExactMatching) {
      if (alias.name != text) {
        continue;
      }
    } else {
      if (!textMatchesEmojiName(text, alias.name, true)) {
        continue;
      }
    }

    EmojiLabel* emojiLabel = nullptr;
    for (const auto& emoji : emojis) {
      if (emoji.code == alias.code) {
        emojiLabel = new EmojiLabel(nullptr, emoji);
        break;
      }
    }

    if (emojiLabel == nullptr) {
      emojiLabel = new EmojiLabel(nullptr, alias);
    }

    emojiLabel->setProperty("alias", QString::fromStdString(alias.name));

    if (addEmojiLabel(emojiLabel, row, col)) {
      break;
    }

    if (aliasExactMatching) {
      if (col != 0) {
        return;
      }
    } else {
      if (row == rows) {
        return;
      }
    }
  }

  if (row == rows) {
    return;
  }

  for (const auto& emoji : emojis) {
    if (isDisabledEmoji(emoji)) {
      continue;
    }

    if (!textMatchesEmojiName(text, tr(emoji.name.data()).toStdString(), false)) {
      continue;
    }

    if (addEmojiLabel(new EmojiLabel(nullptr, emoji), row, col)) {
      break;
    }
  }
}

void EmojiPicker::fillViewWithEmojisByList() {
  _emojiEdit->helpLabel()->setHighlighted(true);

  int col = 0;
  int row = 0;

  int idx = 0;

  int startCol = 0;
  int startRow = 0;

  if (_helpEmojiListIdx != -1) {
    _helpEmojiListStartEmoji = {"", ""};
  }

  if (_helpEmojiListStartEmoji) {
    for (const auto& emoji : emojis) {
      if (isDisabledEmoji(emoji)) {
        continue;
      }

      if (emoji == _helpEmojiListStartEmoji) {
        if (startRow % 2 != 0) {
          startRow -= 1;
        }

        break;
      }

      startCol += 1;

      if (startCol == cols) {
        startCol = 0;
        startRow += 1;
      }
    }
  }

  for (const auto& emoji : emojis) {
    if (isDisabledEmoji(emoji)) {
      continue;
    }

    idx += 1;

    if (_helpEmojiListStartEmoji && (idx - 1) < (startRow * cols)) {
      continue;
    }

    if ((idx - 1) < (_helpEmojiListIdx - (_helpEmojiListDir == -1 ? (rows * cols) : 0))) {
      continue;
    }

    if (addEmojiLabel(new EmojiLabel(nullptr, emoji), row, col)) {
      _helpEmojiListIdx = idx - (2 * cols);

      break;
    }
  }

  if (_helpEmojiListStartEmoji) {
    for (EmojiLabel* emojiLabel : _emojiLayoutWidget->findChildren<EmojiLabel*>()) {
      if (emojiLabel->emoji() == _helpEmojiListStartEmoji) {
        setSelectedEmojiLabel(emojiLabel);
      }
    }
  }
}

void EmojiPicker::onTextChanged(const QString& textQStr) {
  clearView();

  if (textQStr == "") {
    fillViewWithRecentEmojis();
  } else {
    fillViewWithEmojisByText(textQStr.toStdString());
  }
}

void EmojiPicker::onReturnPressed(const QKeyEvent& event) {
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

  bool closeAfter = event.modifiers() & Qt::ShiftModifier;
  if (EmojiPickerSettings::snapshot().swapEnterAndShiftEnter()) {
    closeAfter = !closeAfter;
  }

  emit returnPressed(emoji.code, closeAfter);

  if (closeAfter) {
    emitEscapePressed();
  }
}

void EmojiPicker::onArrowKeyPressed(const QKeyEvent& event) {
  if (_selectedEmojiLabel == nullptr) {
    return;
  }

  int rowToSelect = _selectedEmojiLabel->property("row").toInt();
  int colToSelect = _selectedEmojiLabel->property("col").toInt();

  switch (event.key()) {
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

      return;
    }
  }

  if (_helpEmojiListIdx != -1 && (event.key() == Qt::Key_Up || event.key() == Qt::Key_Down)) {
    Emoji selectedEmoji = _selectedEmojiLabel->emoji();

    if (event.key() == Qt::Key_Down) {
      _helpEmojiListDir = +1;
    } else if (event.key() == Qt::Key_Up) {
      _helpEmojiListDir = -1;
    }

    clearView();
    fillViewWithEmojisByList();

    for (EmojiLabel* emojiLabel : _emojiLayoutWidget->findChildren<EmojiLabel*>()) {
      if (emojiLabel->emoji() == selectedEmoji) {
        setSelectedEmojiLabel(emojiLabel);
      }
    }
  }
}

void EmojiPicker::emitEscapePressed() {
  EmojiPickerSettings().setRecentEmojis(_recentEmojis);

  emit escapePressed();
}

void EmojiPicker::onEscapePressed(const QKeyEvent& event) {
  emitEscapePressed();
}

void EmojiPicker::onFunctionKeyPressed(const QKeyEvent& event) {
  switch (event.key()) {
  case Qt::Key_F1:
    onFavsPressed(nullptr);
    break;
  case Qt::Key_F2:
    onHelpPressed(nullptr);
    break;
  case Qt::Key_F4:
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(_settingsPath)));
    emitEscapePressed();
    break;
  case Qt::Key_F5:
    emit toggleInputMethod();
    break;
  }
}

void EmojiPicker::onTabPressed(const QKeyEvent& event) {
  if (_helpEmojiListIdx == -1) {
    onHelpPressed(nullptr);
  } else if (_helpEmojiListIdx != -1 || _emojiEdit->text() != "") {
    onFavsPressed(nullptr);
  }
}

void EmojiPicker::onCopyPressed(const QKeyEvent& event) {
  if (_selectedEmojiLabel == nullptr) {
    return;
  }

  const Emoji& emoji = _selectedEmojiLabel->emoji();

  QApplication::clipboard()->clear();
  QApplication::clipboard()->setText(QString::fromStdString(emoji.code));
}

void EmojiPicker::onFavsPressed(QMouseEvent* ev) {
  if (_helpEmojiListIdx == -1 && _emojiEdit->text() == "") {
    return;
  }

  _helpEmojiListStartEmoji = {"", ""};
  _helpEmojiListIdx = -1;
  _emojiEdit->setText("");
  clearView();
  fillViewWithRecentEmojis();
}

void EmojiPicker::onHelpPressed(QMouseEvent* ev) {
  if (_helpEmojiListIdx != -1) {
    return;
  }

  if (_emojiEdit->text() != "" && _selectedEmojiLabel != nullptr) {
    _helpEmojiListStartEmoji = _selectedEmojiLabel->emoji();
  }

  _helpEmojiListDir = +1;
  _emojiEdit->setText("");
  clearView();
  fillViewWithEmojisByList();
}
