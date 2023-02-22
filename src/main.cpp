#include "EmojiPickerWindow.hpp"
#include "EmojiPickerSettings.hpp"
#include "EmojiTranslator.hpp"
#include "crossdo.h"
#undef Status
#undef KeyPress
#undef KeyRelease
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QMainWindow>
#include <QMimeData>
#include <QTextStream>
#include <QTimer>
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

QMimeData* copyQMimeData(const QMimeData* source) {
  QMimeData* target = new QMimeData();

  for (QString format : source->formats()) {
    target->setData(format, source->data(format));
  }

  return target;
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

std::string getInputMethod(const std::string& prevWindowProcessName, EmojiPickerSettings& settings) {
  bool activateWindowBeforeWriting = settings.activateWindowBeforeWriting(prevWindowProcessName);
  bool useClipboardHack = settings.useClipboardHack(prevWindowProcessName);

  if (!activateWindowBeforeWriting && !useClipboardHack) {
    return "default";
  } else if (!activateWindowBeforeWriting && useClipboardHack) {
    return "ctrl+v";
  } else if (activateWindowBeforeWriting && !useClipboardHack) {
    return "activate window & default";
  } else {
    return "activate window & ctrl+v";
  }
}

int main(int argc, char** argv) {
  QApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QApplication::setApplicationName(PROJECT_NAME);
  QApplication::setApplicationVersion(PROJECT_VERSION);

  QApplication app(argc, argv);
  QApplication::installTranslator(new EmojiTranslator(nullptr, EmojiPickerSettings::snapshot().localeKey()));

  auto crossdo = std::unique_ptr<crossdo_t, decltype(&crossdo_free)>(crossdo_new(), &crossdo_free);
  window_t prevWindow;
  crossdo_get_active_window(crossdo.get(), &prevWindow);
  int prevWindowPID = crossdo_get_pid_window(crossdo.get(), prevWindow);
  std::string prevWindowProcessName = getProcessNameFromPID(prevWindowPID);

  bool activateWindowBeforeWriting = EmojiPickerSettings::snapshot().activateWindowBeforeWriting(prevWindowProcessName);
  bool useClipboardHack = EmojiPickerSettings::snapshot().useClipboardHack(prevWindowProcessName);

  if (!EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    app.setStyle("fusion");
    app.setStyleSheet(readQFileIfExists(":/main.qss"));
  }

  EmojiPickerWindow* window = new EmojiPickerWindow();

  if (!EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    window->setStyleSheet(readQFileIfExists(QString::fromStdString(EmojiPickerSettings::snapshot().customQssFilePath())));
  }

  QMimeData* prevClipboardMimeData = nullptr;

  window->statusBar()->showMessage(QString::fromStdString(
      std::string("Mode: ") + getInputMethod(prevWindowProcessName, EmojiPickerSettings::snapshot())));

  window->commitText = [&](const std::string& text, bool closeAfter) {
    window_t currentWindow = 0;
    if (activateWindowBeforeWriting || closeAfter) {
      crossdo_get_active_window(crossdo.get(), &currentWindow);
    }

    if (currentWindow != 0) {
      crossdo_activate_window(crossdo.get(), prevWindow);
      crossdo_wait_for_window_active(crossdo.get(), prevWindow, 1);
    }

    if (useClipboardHack) {
      if (prevClipboardMimeData == nullptr) {
        prevClipboardMimeData = copyQMimeData(QApplication::clipboard()->mimeData());
      }

      QApplication::clipboard()->clear();
      QApplication::clipboard()->setText(QString::fromStdString(text));

      if (closeAfter) {
#ifdef __linux__
        charcodemap_t* keys;
        int keysLen;

        xdo_get_active_modifiers(crossdo.get(), &keys, &keysLen);
        xdo_clear_active_modifiers(crossdo.get(), prevWindow, keys, keysLen);
#endif
      }

      crossdo_send_keysequence_window(crossdo.get(), prevWindow, "ctrl+v", 12000);
    } else {
      crossdo_enter_text_window(crossdo.get(), prevWindow, text.data(), 12000);
    }

    if (currentWindow != 0 && !closeAfter) {
      crossdo_activate_window(crossdo.get(), currentWindow);
      crossdo_wait_for_window_active(crossdo.get(), currentWindow, 1);
    }
  };

  window->onDisable = [&]() {
    EmojiPickerSettings::writeDefaultsToDisk();

    if (useClipboardHack && prevClipboardMimeData != nullptr) {
      window->hide();

      QTimer::singleShot(100, [&]() {
        QApplication::clipboard()->clear();
        QApplication::clipboard()->setMimeData(prevClipboardMimeData);

        QObject::connect(QApplication::clipboard(), &QClipboard::dataChanged, [&]() {
          app.exit();
        });
      });
    } else {
      app.exit();
    }
  };

  window->onToggleInputMode = [&]() {
    EmojiPickerSettings settings;
    settings.toggleInputMethod(prevWindowProcessName);

    activateWindowBeforeWriting = settings.activateWindowBeforeWriting(prevWindowProcessName);
    useClipboardHack = settings.useClipboardHack(prevWindowProcessName);

    window->statusBar()->showMessage(
        QString::fromStdString(std::string("Mode: ") + getInputMethod(prevWindowProcessName, settings)));
  };

  window->enable();

  if (EmojiPickerSettings::snapshot().openAtMouseLocation()) {
    window->move(QCursor::pos());
  }

  window->show();

  return app.exec();
}
