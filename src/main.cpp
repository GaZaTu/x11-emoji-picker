#include "EmojiPicker.hpp"
#include <QApplication>
#include <QMainWindow>

extern "C" {
#include "xdo.h"
}

int main(int argc, char** argv) {
  xdo_t* xdo = xdo_new(nullptr);
  Window prevX11Window;
  xdo_get_active_window(xdo, &prevX11Window);

  QApplication::setOrganizationName("gazatu.xyz");
  QApplication::setOrganizationDomain("gazatu.xyz");
  QApplication::setApplicationName("emoji-picker");
  QApplication::setApplicationVersion("0.2.0");

  QApplication app(argc, argv);
  QMainWindow window;
  window.resize(358, 192);
  window.setWindowOpacity(0.90);
  window.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  window.setWindowIcon(QIcon(":/res/72x72/1f0cf.png"));

  EmojiPicker* mainWidget = new EmojiPicker();

  QObject::connect(mainWidget, &EmojiPicker::returnPressed, [&](const std::string& emojiStr) {
    xdo_enter_text_window(xdo, prevX11Window, emojiStr.data(), 0);
  });

  QObject::connect(mainWidget, &EmojiPicker::escapePressed, [&]() {
    app.exit();
  });

  window.setCentralWidget(mainWidget);
  window.show();

  return app.exec();
}
