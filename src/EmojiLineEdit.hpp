#pragma once

#include <QKeyEvent>
#include <QLineEdit>

class EmojiLineEdit : public QLineEdit {
  Q_OBJECT

public:
  explicit EmojiLineEdit(QWidget* parent = nullptr);

Q_SIGNALS:
  void arrowKeyPressed(int key);
  void escapePressed();

protected:
  void keyPressEvent(QKeyEvent* event) override;
};
