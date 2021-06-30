#pragma once

#include "EmojiPicker.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QMimeData>
#include <memory>

namespace app {
struct args {
  bool runAsDaemon = false;

  args(QCoreApplication& app);
};

class main {
public:
  QApplication& app;
  QMainWindow window;
  EmojiPicker* mainWidget;

  int uinputFd = 0;

  std::string prevWindowProcessName = "";
  bool activateWindowBeforeWriting = false;
  bool useClipboardHack = false;
  std::string inputMethod = "";

  uint64_t prevWindow = 0;
  int prevWindowPID = 0;

  QMimeData* prevClipboardMimeData = nullptr;

  main(QApplication& app, args& args);

  void show();

  void hide();

private:
  void initState();
};
} // namespace app
