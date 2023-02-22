#include "EmojiPickerWindow.hpp"
#include "EmojiLabel.hpp"
#include "emojis.hpp"
#include "kaomojis.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <algorithm>
#include <exception>
#include <memory>
#include <vector>

Emoji convertKaomojiToEmoji(const Kaomoji& kaomoji) {
  return Emoji{kaomoji.name, kaomoji.text, -1};
}

void moveQWidgetToCenter(QWidget* window) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  auto rect = QApplication::screenAt(QCursor::pos())->geometry();
#else
  auto rect = QApplication::desktop()->availableGeometry(QCursor::pos());
#endif

  window->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, window->size(), rect));
}

void moveQWidgetToPoint(QWidget* window, QPoint windowPoint) {
  QRect windowRect = window->geometry();

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  QRect screenRect = QApplication::screenAt(windowPoint)->geometry();
#else
  QRect screenRect = QApplication::desktop()->availableGeometry(windowPoint);
#endif

  if ((windowPoint.x() + windowRect.width()) > (screenRect.x() + screenRect.width())) {
    windowPoint.setX(windowPoint.x() - windowRect.width());
  }

  if ((windowPoint.y() + windowRect.height()) > (screenRect.y() + screenRect.height())) {
    windowPoint.setY(windowPoint.y() - windowRect.height());
  }

  window->move(windowPoint);
}

EmojiPickerWindow::EmojiPickerWindow() : QMainWindow() {
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  setWindowIcon(QIcon(":/res/x11-emoji-picker.png"));
  setWindowOpacity(_settings.windowOpacity());
  setFixedSize(340, 190);

  _searchContainerWidget->setLayout(_searchContainerLayout);
  _searchContainerLayout->setStackingMode(QStackedLayout::StackAll);

  _searchCompletion->setFocusPolicy(Qt::NoFocus);
  _searchCompletion->setReadOnly(true);
  if (EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    _searchCompletion->setStyleSheet(_searchCompletion->styleSheet() + QString("background: #00000000;"));
    QColor _searchCompletionTextColor = _searchEdit->palette().text().color();
    _searchCompletionTextColor.setAlphaF(0.6);
    _searchCompletion->setStyleSheet(
        _searchCompletion->styleSheet() + QString("color: #%1;").arg(_searchCompletionTextColor.rgba(), 0, 16));
  } else {
    _searchCompletion->setStyleSheet(_searchCompletion->styleSheet() + QString("background: #00000000;"));
    _searchCompletion->setStyleSheet(_searchCompletion->styleSheet() + QString("color: rgba(255, 255, 255, 155);"));
  }

  _searchContainerLayout->addWidget(_searchCompletion);
  _searchContainerLayout->addWidget(_searchEdit);

  _emojiListWidget->setFocusPolicy(Qt::NoFocus);
  _emojiListWidget->setLayout(_emojiListLayout);
  _emojiListLayout->setContentsMargins(4, 4, 4, 4);
  _emojiListLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  _emojiListLayout->setAlignment(Qt::AlignTop);

  _emojiListScroll->setFocusPolicy(Qt::NoFocus);
  _emojiListScroll->setWidgetResizable(true);
  _emojiListScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  _emojiListScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  _emojiListScroll->setWidget(_emojiListWidget);

  _centralWidget->setFocusPolicy(Qt::NoFocus);
  _centralWidget->setLayout(_centralLayout);
  _centralLayout->setContentsMargins(0, 0, 0, 0);
  _centralLayout->setSpacing(0);
  _centralLayout->addWidget(_searchContainerWidget);
  _centralLayout->addWidget(_emojiListScroll, 1);

  setCentralWidget(_centralWidget);

  _mruModeLabel->setEmoji({"favorites", u8"⭐"}, 14, 14);
  _mruModeLabel->setHighlighted(_mode == ViewMode::MRU);
  _listModeLabel->setEmoji({"emoji list", u8"🗃"}, 14, 14);
  _listModeLabel->setHighlighted(_mode == ViewMode::LIST);
  _kaomojiModeLabel->setEmoji({"kaomoji list", u8"ヽ(o^ ^o)ﾉ"}, 18, 18);
  _kaomojiModeLabel->setHighlighted(_mode == ViewMode::KAOMOJI);

  _statusBar->setFixedHeight(20);
  _statusBar->addPermanentWidget(_mruModeLabel);
  _statusBar->addPermanentWidget(_listModeLabel);
  _statusBar->addPermanentWidget(_kaomojiModeLabel);

  setStatusBar(_statusBar);

  QObject::connect(_searchEdit, &EmojiLineEdit::updateEmojiList, this, &EmojiPickerWindow::updateEmojiList);
  QObject::connect(_searchEdit, &EmojiLineEdit::processKeyEvent, this, &EmojiPickerWindow::processKeyEvent);
  QObject::connect(_searchEdit, &EmojiLineEdit::disable, this, &EmojiPickerWindow::disable);

  _emojiLayoutItems.reserve((sizeof(emojis) / sizeof(Emoji)) + (sizeof(kaomojis) / sizeof(Kaomoji)));
}

void EmojiPickerWindow::wheelEvent(QWheelEvent* event) {
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

  if (stepsDelta.y() > 0) {
    moveSelectedEmojiLabel(-1, 0);
  } else {
    moveSelectedEmojiLabel(1, 0);
  }
}

EmojiLabel* EmojiPickerWindow::selectedEmojiLabel() {
  QLayoutItem* item = _emojiListLayout->itemAtPosition(_selectedRow, _selectedColumn);
  if (!item) {
    return nullptr;
  }

  EmojiLabel* widget = static_cast<EmojiLabel*>(item->widget());
  return widget;
}

void showEmojiRow(QGridLayout* _emojiListLayout, int row) {
  for (int column = 0; column < _emojiListLayout->columnCount(); column++) {
    QLayoutItem* emojiLayoutItem = _emojiListLayout->itemAtPosition(row, column);
    if (!emojiLayoutItem) {
      continue;
    }

    auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());
    label->show();
  }
}

void EmojiPickerWindow::moveSelectedEmojiLabel(int row, int column) {
  EmojiLabel* previousLabel = selectedEmojiLabel();

  if (previousLabel) {
    previousLabel->setHighlighted(false);
  }

  _selectedRow += row;
  _selectedColumn += column;

  EmojiLabel* nextLabel = selectedEmojiLabel();

  // the following is a piece of shit
  // basically kaomoji have a colspan of 2 which means:
  // moving left or right requires a 2 instead of 1
  if (nextLabel == previousLabel) {
    _selectedRow += row;
    _selectedColumn += column;

    nextLabel = selectedEmojiLabel();
  }

  if (!nextLabel || nextLabel->emoji().code == "") {
    _selectedRow -= row;
    _selectedColumn -= column;

    nextLabel = selectedEmojiLabel();
  }

  if (nextLabel) {
    nextLabel->setHighlighted(true);

    if (row != 0) {
      for (int x = _selectedRow; x < std::min(_selectedRow + 5, _emojiListLayout->rowCount()); x++) {
        showEmojiRow(_emojiListLayout, x);
      }
      for (int x = _selectedRow; x >= std::max(_selectedRow - 5, 0); x--) {
        showEmojiRow(_emojiListLayout, x);
      }
    }

    _emojiListScroll->ensureWidgetVisible(nextLabel);

    updateSearchCompletion();
  }
}

bool stringIncludes(const std::string& text, const std::string& search) {
  auto found = std::search(text.begin(), text.end(), search.begin(), search.end(), [](char c1, char c2) {
    return std::tolower(c1) == std::tolower(c2);
  });

  if (search.length() >= 3) {
    return found != text.end();
  } else {
    return found == text.begin();
  }
}

bool EmojiPickerWindow::stringMatches(const QString& target, const QString& search, SearchMode mode) {
  if (mode == SearchMode::AUTO) {
    if (search.length() < 3) {
      mode = SearchMode::STARTS_WITH;
    } else {
      mode = SearchMode::CONTAINS;
    }
  }

  switch (mode) {
  case SearchMode::CONTAINS:
    return target.contains(search, Qt::CaseInsensitive);

  case SearchMode::STARTS_WITH:
    return target.startsWith(search, Qt::CaseInsensitive);

  case SearchMode::EQUALS:
    return QString::compare(target, search, Qt::CaseInsensitive) == 0;

  default:
    return false;
  }
}

bool EmojiPickerWindow::emojiMatchesSearch(const Emoji& emoji, const QString& search, SearchMode mode, QString& found) {
  QString emojiName = tr(emoji.name.data());

  if (stringMatches(emojiName, search, mode)) {
    found = emojiName;
    return true;
  }

  for (const QString& alias : _emojiAliases[emoji.code]) {
    if (stringMatches(alias, search, mode)) {
      found = alias;
      return true;
    }
  }

  return false;
}

bool EmojiPickerWindow::emojiMatchesSearch(const Emoji& emoji, const QString& search, SearchMode mode) {
  QString found;

  return emojiMatchesSearch(emoji, search, mode, found);
}

void EmojiPickerWindow::updateSearchCompletion() {
  QString search = _searchEdit->text();

  QString completion = "";
  if (selectedEmojiLabel()) {
    const Emoji& emoji = selectedEmojiLabel()->emoji();

    emojiMatchesSearch(emoji, search, SearchMode::AUTO, completion);
  }

  int indexOfSearch = std::max(completion.indexOf(search, 0, Qt::CaseInsensitive), 0);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  int offsetOfSearch = _searchEdit->fontMetrics().horizontalAdvance(completion.left(indexOfSearch));
#else
  int offsetOfSearch = _searchEdit->fontMetrics().width(completion.left(indexOfSearch));
#endif

  if (completion != "" && !_searchEdit->hasSelectedText()) {
    _searchEdit->setText(completion.mid(indexOfSearch, search.length()));
  }

  _searchEdit->setTextMargins(offsetOfSearch, 0, 0, 0);

  _searchCompletion->setText(completion);
}

void EmojiPickerWindow::addItemToEmojiList(
    QLayoutItem* emojiLayoutItem, EmojiLabel* label, int colspan, int& row, int& column) {
  if (colspan == 0) {
    if (label->hasRealEmoji()) {
      colspan = 1;
    } else {
      colspan = 2;
    }
  }

  if ((column + colspan) > _rowSize) {
    column = 0;
    row += 1;
  }

  _emojiListLayout->addItem(emojiLayoutItem, row, column, 1, colspan, Qt::AlignHCenter | Qt::AlignBaseline);

  column += colspan;
  if (column >= _rowSize) {
    column = 0;
    row += 1;
  }
}

QWidgetItem* EmojiPickerWindow::createEmojiLabel(const Emoji& emoji) {
  auto existing = _emojiLayoutItems[emoji.code];
  if (!existing) {
    auto emojiLayoutWidget = new EmojiLabel(_emojiListWidget, _settings, emoji);
    auto emojiLayoutItem = new QWidgetItemV2(emojiLayoutWidget);

    _emojiLayoutItems[emoji.code] = existing = emojiLayoutItem;
    emojiLayoutWidget->hide();

    QObject::connect(emojiLayoutWidget, &EmojiLabel::mousePressed, [this, emojiLayoutWidget]() {
      commitEmoji(emojiLayoutWidget->emoji(), emojiLayoutWidget->hasRealEmoji(), _settings.swapEnterAndShiftEnter());
    });
  }

  return existing;
}

QWidgetItem* EmojiPickerWindow::getEmojiLayoutItem(const Emoji& emoji) {
  return createEmojiLabel(emoji);
}

QWidgetItem* EmojiPickerWindow::getKaomojiLayoutItem(const Kaomoji& kaomoji) {
  return createEmojiLabel(convertKaomojiToEmoji(kaomoji));
}

void EmojiPickerWindow::updateEmojiList() {
  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(false);
  }

  QLayoutItem* itemToRemove;
  while ((itemToRemove = _emojiListLayout->itemAt(0))) {
    _emojiListLayout->removeItem(itemToRemove);

    auto label = static_cast<EmojiLabel*>(itemToRemove->widget());
    label->hide();
  }

  QString search = _searchEdit->text();
  std::string searchAsStdString = search.toStdString();

  int row = 0;
  int column = 0;

  _emojiListLayout->setEnabled(false);

  switch (_mode) {
  case ViewMode::MRU: {
    if (search == "") {
      for (const auto& emoji : _emojiMRU) {
        auto emojiLayoutItem = getEmojiLayoutItem(emoji);
        auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());

        label->show();

        addItemToEmojiList(&*emojiLayoutItem, label, 0, row, column);
      }
      break;
    }
    // fallthrough to ViewMode::LIST if search != ""
  }

  case ViewMode::LIST: {
    std::unordered_set<std::string> addedEmojis; // std::string_view would be better
    auto addEmojis = [&](SearchMode searchMode) {
      for (const auto& emoji : emojis) {
        if (addedEmojis.count(emoji.code) != 0) {
          continue;
        }

        if (_disabledEmojis.count(emoji.code) != 0) {
          continue;
        }

        if (search != "" && !emojiMatchesSearch(emoji, search, searchMode)) {
          continue;
        }

        auto emojiLayoutItem = getEmojiLayoutItem(emoji);
        auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());

        if (row <= 5) {
          label->show();
        }

        addItemToEmojiList(&*emojiLayoutItem, label, 0, row, column);

        addedEmojis.emplace(emoji.code);

        if (search != "" && row >= 5) {
          break;
        }
      }
    };

    if (search != "") {
      addEmojis(SearchMode::EQUALS);
      addEmojis(SearchMode::STARTS_WITH);
    }
    addEmojis(SearchMode::AUTO);
    break;
  }

  case ViewMode::KAOMOJI: {
    for (const auto& kaomoji : kaomojis) {
      if (_disabledEmojis.count(kaomoji.text) != 0) {
        continue;
      }

      if (search != "" && !stringIncludes(kaomoji.name, searchAsStdString)) {
        continue;
      }

      auto emojiLayoutItem = getKaomojiLayoutItem(kaomoji);
      auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());

      if (row <= 6) {
        label->show();
      }

      addItemToEmojiList(&*emojiLayoutItem, label, 0, row, column);

      if (search != "" && row >= 5) {
        break;
      }
    }
    break;
  }
  }

  _selectedRow = 0;
  _selectedColumn = 0;
  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(true);

    // add dummy labels so real labels keep their correct position to the left (NOT the same as Qt::AlignLeft)
    while (row == 0) {
      auto emojiLayoutItem = getEmojiLayoutItem(Emoji{"__DUMMY" + std::to_string(column), ""});
      auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());

      label->show();

      addItemToEmojiList(&*emojiLayoutItem, label, (_mode == ViewMode::KAOMOJI) ? 2 : 1, row, column);
    }
  }

  _emojiListLayout->setEnabled(true);

  updateSearchCompletion();
}

void EmojiPickerWindow::reset() {
  disable();
}

void EmojiPickerWindow::enable() {
  moveQWidgetToCenter(this);

  show();

  _settings.sync();

  _disabledEmojis.clear();
  for (const Emoji& emoji : emojis) {
    if (_settings.isDisabledEmoji(emoji, fontMetrics())) {
      _disabledEmojis.insert(emoji.code);
    }
  }

  _emojiAliases = _settings.emojiAliases();

  _emojiMRU = EmojiPickerCache{}.emojiMRU();
}

void EmojiPickerWindow::changeEvent(QEvent* event) {
  QWidget::changeEvent(event);

  if (event->type() == QEvent::ActivationChange) {
    if (isVisible()) {
      _searchEdit->setText("");
      _searchCompletion->setText("");
      updateEmojiList();
    }
  }
}

void EmojiPickerWindow::disable() {
  if (!isVisible()) {
    return;
  }

  EmojiPickerCache{}.emojiMRU(_emojiMRU);

  onDisable();
}

void EmojiPickerWindow::setCursorLocation(const QRect* rect) {
  if (rect->x() == 0 && rect->y() == 0) {
    return;
  }

  move(rect->x(), rect->y() + rect->height());
}

void EmojiPickerWindow::commitEmoji(const Emoji& emoji, bool isRealEmoji, bool closeAfter) {
  commitText(emoji.code, closeAfter);

  if (isRealEmoji || _settings.saveKaomojiInMRU()) {
    _emojiMRU.erase(std::remove(_emojiMRU.begin(), _emojiMRU.end(), emoji), _emojiMRU.end());
    _emojiMRU.insert(_emojiMRU.begin(), emoji);
    while (_emojiMRU.size() > 40) {
      _emojiMRU.pop_back();
    }
  }

  if (closeAfter) {
    disable();
  }
}

void EmojiPickerWindow::processKeyEvent(const QKeyEvent* event) {
  EmojiAction action = getEmojiActionForQKeyEvent(event);

  switch (action) {
  case EmojiAction::INVALID:
    break;

  case EmojiAction::COPY_SELECTED_EMOJI:
    if (selectedEmojiLabel()) {
      const EmojiLabel* label = selectedEmojiLabel();
      const Emoji& emoji = label->emoji();

      QApplication::clipboard()->setText(QString::fromStdString(emoji.code));
    }
    break;

  case EmojiAction::DISABLE:
    disable();
    break;

  case EmojiAction::COMMIT_EMOJI:
    if (selectedEmojiLabel()) {
      bool closeAfter = (((event->modifiers() & Qt::ShiftModifier) && !_settings.swapEnterAndShiftEnter()) ||
          (!(event->modifiers() & Qt::ShiftModifier) && _settings.swapEnterAndShiftEnter()));

      commitEmoji(selectedEmojiLabel()->emoji(), selectedEmojiLabel()->hasRealEmoji(), closeAfter);
    }
    break;

  case EmojiAction::SWITCH_VIEW_MODE:
    if (event->modifiers() & Qt::ShiftModifier) {
      switch (_mode) {
      case ViewMode::MRU:
        _mode = ViewMode::KAOMOJI;
        break;
      case ViewMode::LIST:
        _mode = ViewMode::MRU;
        break;
      case ViewMode::KAOMOJI:
        _mode = ViewMode::LIST;
        break;
      }
    } else {
      switch (_mode) {
      case ViewMode::MRU:
        _mode = ViewMode::LIST;
        break;
      case ViewMode::LIST:
        _mode = ViewMode::KAOMOJI;
        break;
      case ViewMode::KAOMOJI:
        _mode = ViewMode::MRU;
        break;
      }
    }
    _mruModeLabel->setHighlighted(_mode == ViewMode::MRU);
    _listModeLabel->setHighlighted(_mode == ViewMode::LIST);
    _kaomojiModeLabel->setHighlighted(_mode == ViewMode::KAOMOJI);
    updateEmojiList();
    break;

  case EmojiAction::UP:
    moveSelectedEmojiLabel(-1, 0);
    break;

  case EmojiAction::DOWN:
    moveSelectedEmojiLabel(1, 0);
    break;

  case EmojiAction::LEFT:
    moveSelectedEmojiLabel(0, -1);
    break;

  case EmojiAction::RIGHT:
    moveSelectedEmojiLabel(0, 1);
    break;

  case EmojiAction::PAGE_UP:
    moveSelectedEmojiLabel(-4, 0);
    break;

  case EmojiAction::PAGE_DOWN:
    moveSelectedEmojiLabel(4, 0);
    break;

  case EmojiAction::OPEN_SETTINGS:
    QDesktopServices::openUrl(QUrl::fromLocalFile(_settings.fileName()));
    disable();
    break;

  case EmojiAction::TOGGLE_INPUT_MODE:
    onToggleInputMode();
    break;
  }
}
