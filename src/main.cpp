#include "EmojiPicker.hpp"
#include "EmojiPickerSettings.hpp"
#include "EmojiTranslator.hpp"
#include "crossdo.h"
#undef Status
#include <QApplication>
#include <QMainWindow>
#include <QTextStream>
#include <memory>

QString readQFileIfExists(const QString& path) {
  QFile qssFile(path);
  if (!qssFile.exists()) {
    return "";
  }

  qssFile.open(QFile::ReadOnly | QFile::Text);
  QTextStream ts(&qssFile);
  return ts.readAll();
}

int main(int argc, char** argv) {
  auto crossdo = std::unique_ptr<crossdo_t, decltype(&crossdo_free)>(crossdo_new(), &crossdo_free);
  window_t prevWindow;
  crossdo_get_active_window(crossdo.get(), &prevWindow);

  QApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QApplication::setApplicationName(PROJECT_NAME);
  QApplication::setApplicationVersion(PROJECT_VERSION);

  QApplication app(argc, argv);
  QApplication::installTranslator(new EmojiTranslator(nullptr, EmojiPickerSettings::startupSnapshot().localeKey()));

  EmojiPickerSettings::writeDefaultsToDisk();

  if (!EmojiPickerSettings::startupSnapshot().useSystemQtTheme()) {
    app.setStyle("fusion");
    app.setStyleSheet(readQFileIfExists(":/main.qss"));
  }

  QMainWindow window;

  if (EmojiPickerSettings::startupSnapshot().useSystemQtTheme()) {
    window.resize(358, 192);
  } else {
    window.resize(370, 206);
  }

  window.setWindowOpacity(0.90);
  window.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  window.setWindowIcon(QIcon(":/res/72x72/1f0cf.png"));

  if (!EmojiPickerSettings::startupSnapshot().useSystemQtTheme()) {
    window.setStyleSheet(readQFileIfExists(QString::fromStdString(EmojiPickerSettings::startupSnapshot().customQssFilePath())));
  }

  if (EmojiPickerSettings::startupSnapshot().openAtMouseLocation()) {
    int cursorX = 0;
    int cursorY = 0;
    crossdo_get_mouse_location2(crossdo.get(), &cursorX, &cursorY, nullptr, nullptr);

    window.move(cursorX, cursorY);
  }

  EmojiPicker* mainWidget = new EmojiPicker();

  QObject::connect(mainWidget, &EmojiPicker::returnPressed, [&](const std::string& emojiStr) {
    crossdo_enter_text_window(crossdo.get(), prevWindow, emojiStr.data(), 12000);
  });

  QObject::connect(mainWidget, &EmojiPicker::escapePressed, [&]() {
    app.exit();
  });

  window.setCentralWidget(mainWidget);
  window.show();

  return app.exec();
}
