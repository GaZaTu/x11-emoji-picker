#pragma once

#include "EmojiLabel.hpp"
#include "EmojiPickerSettings.hpp"
#include <QGridLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMainWindow>
#include <QScrollArea>
#include <QStackedLayout>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "EmojiLineEdit.hpp"
#include "kaomojis.hpp"

struct EmojiPickerWindow : public QMainWindow {
  Q_OBJECT

public:
  std::function<void(const std::string&, bool)> commitText;
  std::function<void()> onDisable;
  std::function<void()> onToggleInputMode;

  explicit EmojiPickerWindow();

  void updateSearchCompletion();
  void updateEmojiList();

public Q_SLOTS:
  void reset();
  void enable();
  void disable();
  void setCursorLocation(const QRect* rect);
  void processKeyEvent(const QKeyEvent* event);

protected:
  void wheelEvent(QWheelEvent* event) override;

private:
  int _rowSize = 10;

  EmojiPickerSettings _settings;

  QWidgetItem* createEmojiLabel(std::unordered_map<std::string, QWidgetItem*>& layoutItems, const Emoji& emoji);

  std::unordered_map<std::string, QWidgetItem*> _emojiLayoutItems;
  QWidgetItem* getEmojiLayoutItem(const Emoji& emoji);

  std::unordered_map<std::string, QWidgetItem*> _kaomojiLayoutItems;
  QWidgetItem* getKaomojiLayoutItem(const Kaomoji& kaomoji);

  int _selectedRow = 0;
  int _selectedColumn = 0;
  EmojiLabel* selectedEmojiLabel();
  void moveSelectedEmojiLabel(int row, int col);

  QWidget* _centralWidget = new QWidget(this);
  QVBoxLayout* _centralLayout = new QVBoxLayout(_centralWidget);

  QWidget* _searchContainerWidget = new QWidget(_centralWidget);
  QStackedLayout* _searchContainerLayout = new QStackedLayout(_searchContainerWidget);
  EmojiLineEdit* _searchEdit = new EmojiLineEdit(_searchContainerWidget);
  QLineEdit* _searchCompletion = new QLineEdit(_searchContainerWidget);

  QScrollArea* _emojiListScroll = new QScrollArea(_centralWidget);
  QWidget* _emojiListWidget = new QWidget(_emojiListScroll);
  QGridLayout* _emojiListLayout = new QGridLayout(_emojiListWidget);

  QStatusBar* _statusBar = new QStatusBar(this);
  EmojiLabel* _mruModeLabel = new EmojiLabel(_statusBar, _settings);
  EmojiLabel* _listModeLabel = new EmojiLabel(_statusBar, _settings);
  EmojiLabel* _kaomojiModeLabel = new EmojiLabel(_statusBar, _settings);

  void addItemToEmojiList(QLayoutItem* emojiLayoutItem, EmojiLabel* label, int colspan, int& row, int& column);

  enum class SearchMode {
    AUTO,
    CONTAINS,
    STARTS_WITH,
    EQUALS,
  };

  static bool stringMatches(const QString& target, const QString& search, SearchMode mode);

  bool emojiMatchesSearch(const Emoji& emoji, const QString& search, SearchMode mode, QString& found);
  bool emojiMatchesSearch(const Emoji& emoji, const QString& search, SearchMode mode);

  std::unordered_set<std::string> _disabledEmojis;

  std::unordered_map<std::string, std::vector<QString>> _emojiAliases;

  std::vector<Emoji> _emojiMRU;

  enum class ViewMode {
    MRU,
    LIST,
    KAOMOJI,
  };

  ViewMode _mode = ViewMode::MRU;

  void commitEmoji(const Emoji& emoji, bool isRealEmoji, bool closeAfter);
};
