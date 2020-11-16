#include "EmojiPicker.hpp"
#include "EmojiPickerSettings.hpp"
#include "EmojiTranslator.hpp"
#include "crossdo.h"
#undef Status
#include <QApplication>
#include <QClipboard>
#include <QMainWindow>
#include <QTextStream>
#include <memory>

std::string getProcessNameFromPID(int pid) {
  if (pid == 0) {
    return "";
  }

#ifdef __linux__
  QFile qssFile(QString("/proc/%1/status").arg(pid));
  if (!qssFile.exists()) {
    return "";
  }

  qssFile.open(QFile::ReadOnly | QFile::Text);
  QTextStream ts(&qssFile);
  return ts.readLine().remove(0, 6).toStdString();
#elif _WIN32
  return "";
#endif
}

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
  QApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QApplication::setApplicationName(PROJECT_NAME);
  QApplication::setApplicationVersion(PROJECT_VERSION);

  QApplication app(argc, argv);
  QApplication::installTranslator(new EmojiTranslator(nullptr, EmojiPickerSettings::startupSnapshot().localeKey()));

  auto crossdo = std::unique_ptr<crossdo_t, decltype(&crossdo_free)>(crossdo_new(), &crossdo_free);
  window_t prevWindow;
  crossdo_get_active_window(crossdo.get(), &prevWindow);
  int prevWindowPID = crossdo_get_pid_window(crossdo.get(), prevWindow);
  std::string prevWindowProcessName = getProcessNameFromPID(prevWindowPID);

  bool isActivateWindowBeforeWritingException = false;
  for (const auto& exception : EmojiPickerSettings::startupSnapshot().activateWindowBeforeWritingExceptions()) {
    if (exception == prevWindowProcessName) {
      isActivateWindowBeforeWritingException = true;
      break;
    }
  }
  bool activateWindowBeforeWriting =
      ((EmojiPickerSettings::startupSnapshot().activateWindowBeforeWritingByDefault() == true &&
           isActivateWindowBeforeWritingException == false) ||
          (EmojiPickerSettings::startupSnapshot().activateWindowBeforeWritingByDefault() == false &&
              isActivateWindowBeforeWritingException == true));

  bool isCopyEmojiToClipboardAswellException = false;
  for (const auto& exception : EmojiPickerSettings::startupSnapshot().copyEmojiToClipboardAswellExceptions()) {
    if (exception == prevWindowProcessName) {
      isCopyEmojiToClipboardAswellException = true;
      break;
    }
  }
  bool copyEmojiToClipboardAswell =
      ((EmojiPickerSettings::startupSnapshot().copyEmojiToClipboardAswellByDefault() == true &&
           isCopyEmojiToClipboardAswellException == false) ||
          (EmojiPickerSettings::startupSnapshot().copyEmojiToClipboardAswellByDefault() == false &&
              isCopyEmojiToClipboardAswellException == true));

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
    window.setStyleSheet(
        readQFileIfExists(QString::fromStdString(EmojiPickerSettings::startupSnapshot().customQssFilePath())));
  }

  if (EmojiPickerSettings::startupSnapshot().openAtMouseLocation()) {
    int cursorX = 0;
    int cursorY = 0;
    crossdo_get_mouse_location2(crossdo.get(), &cursorX, &cursorY, nullptr, nullptr);

    if (cursorX != 0 && cursorY != 0) {
      window.move(cursorX, cursorY);
    }
  }

  EmojiPicker* mainWidget = new EmojiPicker();

  QObject::connect(mainWidget, &EmojiPicker::returnPressed, [&](const std::string& emojiStr, bool closeAfter) {
    window_t currentWindow = 0;
    if (activateWindowBeforeWriting || closeAfter) {
      crossdo_get_active_window(crossdo.get(), &currentWindow);
    }

    if (currentWindow != 0) {
      crossdo_activate_window(crossdo.get(), prevWindow);
      crossdo_wait_for_window_active(crossdo.get(), prevWindow, 1);
    }

    crossdo_enter_text_window(crossdo.get(), prevWindow, emojiStr.data(), 12000);

    if (currentWindow != 0 && !closeAfter) {
      crossdo_activate_window(crossdo.get(), currentWindow);
      crossdo_wait_for_window_active(crossdo.get(), currentWindow, 1);
    }

    if (copyEmojiToClipboardAswell) {
      QApplication::clipboard()->clear();
      QApplication::clipboard()->setText(QString::fromStdString(emojiStr));
    }

    if (closeAfter) {
      EmojiPickerSettings::writeDefaultsToDisk();

      app.exit();
    }
  });

  QObject::connect(mainWidget, &EmojiPicker::escapePressed, [&]() {
    EmojiPickerSettings::writeDefaultsToDisk();

    app.exit();
  });

  window.setCentralWidget(mainWidget);
  window.show();

  return app.exec();
}
