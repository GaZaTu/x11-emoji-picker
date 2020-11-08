#pragma once

#include "EmojiLabel.hpp"
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QStackedLayout>

class EmojiLineEdit : public QLineEdit {
  Q_OBJECT

public:
  explicit EmojiLineEdit(QWidget* parent = nullptr);

  QWidget* containerWidget();
  QLabel* previewLabel();
  EmojiLabel* favsLabel();
  EmojiLabel* helpLabel();

  std::string previewText();
  void setPreviewText(const std::string& previewText);

signals:
  void arrowKeyPressed(int key);
  void escapePressed();
  void functionKeyPressed(int key);
  void tabPressed();

protected:
  void keyPressEvent(QKeyEvent* event) override;

private:
  QWidget* _container = nullptr;
  QStackedLayout* _containerLayout = nullptr;
  QLabel* _previewLabel = nullptr;
  QWidget* _iconsLayoutWidget = nullptr;
  QGridLayout* _iconsLayout = nullptr;
  EmojiLabel* _favsLabel = nullptr;
  EmojiLabel* _helpLabel = nullptr;
};
