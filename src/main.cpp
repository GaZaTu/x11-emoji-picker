#include "EmojiPicker.hpp"
#include "EmojiPickerSettings.hpp"
#include "EmojiTranslator.hpp"
#include "crossdo.hpp"
#include <QApplication>
#include <QMainWindow>
#include <memory>

void installEmojiTranslator() {
  EmojiPickerSettings settings;
  EmojiTranslator* translator = new EmojiTranslator(nullptr, settings.localeKey());

  QApplication::installTranslator(translator);
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
  installEmojiTranslator();

  QMainWindow window;
  window.resize(358, 192);
  window.setWindowOpacity(0.90);
  window.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  window.setWindowIcon(QIcon(":/res/72x72/1f0cf.png"));

  EmojiPicker* mainWidget = new EmojiPicker();

  QObject::connect(mainWidget, &EmojiPicker::returnPressed, [&](const std::string& emojiStr) {
    crossdo_enter_text_window(crossdo.get(), prevWindow, emojiStr.data(), 0);
  });

  QObject::connect(mainWidget, &EmojiPicker::escapePressed, [&]() {
    app.exit();
  });

  window.setCentralWidget(mainWidget);
  window.show();

  return app.exec();
}
