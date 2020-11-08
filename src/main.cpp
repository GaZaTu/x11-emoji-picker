#include "EmojiPicker.hpp"
#include "EmojiPickerSettings.hpp"
#include "EmojiTranslator.hpp"
#include "crossdo.h"
#undef Status
#include <QApplication>
#include <QMainWindow>
#include <QTextStream>
#include <memory>

int main(int argc, char** argv) {
  auto crossdo = std::unique_ptr<crossdo_t, decltype(&crossdo_free)>(crossdo_new(), &crossdo_free);
  window_t prevWindow;
  crossdo_get_active_window(crossdo.get(), &prevWindow);

  QApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QApplication::setApplicationName(PROJECT_NAME);
  QApplication::setApplicationVersion(PROJECT_VERSION);

  QApplication app(argc, argv);
  QApplication::installTranslator(new EmojiTranslator(nullptr, EmojiPickerSettings().localeKey()));

#ifdef _WIN32
  QFile darkQss(":/main.qss");
  if (darkQss.exists()) {
    darkQss.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&darkQss);
    app.setStyleSheet(ts.readAll());
  }
#endif

  QMainWindow window;

#ifdef __linux__
  window.resize(358, 192);
#elif _WIN32
  window.resize(372, 192);
#endif

  window.setWindowOpacity(0.90);
  window.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  // window.setAttribute(Qt::WA_ShowWithoutActivating);
  window.setWindowIcon(QIcon(":/res/72x72/1f0cf.png"));

  if (EmojiPickerSettings().openAtMouseLocation()) {
    int cursorX = 0;
    int cursorY = 0;
    crossdo_get_mouse_location2(crossdo.get(), &cursorX, &cursorY, NULL, NULL);

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
