#pragma once

#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QStackedLayout>

class EmojiLineEdit : public QLineEdit {
  Q_OBJECT

public:
  explicit EmojiLineEdit(QWidget* parent = nullptr);

  QWidget* containerWidget();

  std::string previewText();
  void setPreviewText(const std::string& previewText);

signals:
  void arrowKeyPressed(int key);
  void escapePressed();

protected:
  void keyPressEvent(QKeyEvent* event) override;

private:
  QWidget* _container = new QWidget();
  QStackedLayout* _containerLayout = new QStackedLayout();
  QLabel* _previewLabel = new QLabel();
};
