#include "EmojiPicker.hpp"
#include "EmojiPickerSettings.hpp"
#include "EmojiTranslator.hpp"
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QMainWindow>
#include <QMimeData>
#include <QTextStream>
#include <QTimer>
#include <memory>
#include "uinput.hpp"
#include <QtDBus>
#include "EmojiPickerDBusInterface.hpp"
#include "WindowManager.hpp"
#include <QDebug>
#include <unistd.h>

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

// struct Subject {
//   QString subject_kind;
//   QMap<QString, QVariant> subject_details;
// };
// Q_DECLARE_METATYPE(Subject);

// QDBusArgument& operator<<(QDBusArgument& argument, const Subject& subject) {
//   argument.beginStructure();
//   argument << subject.subject_kind << subject.subject_details;
//   argument.endStructure();

//   return argument;
// }

// const QDBusArgument& operator>>(const QDBusArgument& argument, Subject&) {
// 	return argument;
// }

// enum class CheckAuthorizationFlags {
//   None = 0x00000000,
//   AllowUserInteraction = 0x00000001,
// };

// struct AuthorizationResult {
//   bool is_authorized;
//   bool is_challenge;
//   QMap<QString, QString> details;
// };
// Q_DECLARE_METATYPE(AuthorizationResult);

// QDBusArgument& operator<<(QDBusArgument& argument, const AuthorizationResult&) {
//   return argument;
// }

// const QDBusArgument& operator>>(const QDBusArgument& argument, AuthorizationResult& result) {
// 	argument.beginStructure();
// 	argument >> result.is_authorized >> result.is_challenge >> result.details;
// 	argument.endStructure();

// 	return argument;
// }

// void requestUInputWritePermission() {
//   qDBusRegisterMetaType<QMap<QString, QVariant>>();
//   qDBusRegisterMetaType<QMap<QString, QString>>();
//   qDBusRegisterMetaType<Subject>();
//   qDBusRegisterMetaType<AuthorizationResult>();

//   Subject subject;
//   subject.subject_kind = "unix-process";
//   subject.subject_details["pid"] = QVariant::fromValue((quint32)getpid());
//   subject.subject_details["start-time"] = QVariant::fromValue((quint64)time(nullptr));
//   QString action_id = "xyz.gazatu.EmojiPicker.uinput";
//   QMap<QString, QString> details;
//   quint32 flags = (quint32)CheckAuthorizationFlags::AllowUserInteraction;
//   QString cancellation_id;

//   QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.PolicyKit1", "/org/freedesktop/PolicyKit1/Authority", "org.freedesktop.PolicyKit1.Authority", "CheckAuthorization");
//   message << QVariant::fromValue(subject) << QVariant::fromValue(action_id) << QVariant::fromValue(details) << QVariant::fromValue(flags) << QVariant::fromValue(cancellation_id);
//   QDBusMessage result = QDBusConnection::systemBus().call(message);

//   qDebug() << result;
// }

int main(int argc, char** argv) {
  QApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QApplication::setApplicationName(PROJECT_NAME);
  QApplication::setApplicationVersion(PROJECT_VERSION);

  QApplication app{argc, argv};

  QCommandLineParser cliParser;
  cliParser.setApplicationDescription("emoji picker");
  cliParser.addHelpOption();
  cliParser.addVersionOption();

  QCommandLineOption daemonOption{"daemon", "run as dbus daemon (service: xyz.gazatu.EmojiPicker, object: /xyz/gazatu/EmojiPicker, methods: show & hide)"};
  cliParser.addOption(daemonOption);

  // QCommandLineOption dbusOption{"dbus", "bus to provide service on (default: $DBUS_SESSION_BUS_ADDRESS)"};
  // cliParser.addOption(dbusOption);

  cliParser.process(app);
  bool runAsDaemon = cliParser.isSet(daemonOption);
  // QString dbusSession = cliParser.value(dbusOption);

  if (runAsDaemon) {
    EmojiPickerSettings::snapshotScope = QSettings::SystemScope;
  }

  QApplication::installTranslator(new EmojiTranslator(nullptr, EmojiPickerSettings::snapshot().localeKey()));

  std::string prevWindowProcessName = "";
  bool activateWindowBeforeWriting = false;
  bool useClipboardHack = false;
  std::string inputMethod = "";

  auto prevWindow = wm::activeWindow();
  auto prevWindowPID = wm::pid(prevWindow);

  qDebug() << "windowManager: " << wm::WindowManager::instance()->name().data();

  uinput::file uinputFile = uinput::open(PROJECT_NAME);

  auto initState = [&]() {
    if (uinputFile) {
      prevWindowProcessName = "";
      activateWindowBeforeWriting = true;
      useClipboardHack = true;
      inputMethod = "uinput & activate window & ctrl+v";
    } else if (prevWindowPID) {
      prevWindowProcessName = getProcessNameFromPID(prevWindowPID);
      activateWindowBeforeWriting = EmojiPickerSettings::snapshot().activateWindowBeforeWriting(prevWindowProcessName);
      useClipboardHack = EmojiPickerSettings::snapshot().useClipboardHack(prevWindowProcessName);
      inputMethod = getInputMethod(prevWindowProcessName, EmojiPickerSettings::snapshot());
    } else {
      prevWindowProcessName = "";
      activateWindowBeforeWriting = true;
      useClipboardHack = true;
      inputMethod = "activate window & ctrl+v";
    }
  };

  initState();

  if (!EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    app.setStyle("fusion");
    app.setStyleSheet(readQFileIfExists(":/main.qss"));
  }

  QMainWindow window;
  int w = 370;
  int h = 236;

  if (EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    w = 358;

    if (EmojiPickerSettings::snapshot().hideInputMethod()) {
      h = 196;
    } else {
      h = 218;
    }
  } else {
    if (EmojiPickerSettings::snapshot().hideInputMethod()) {
      h = 210;
    }
  }

  window.resize(w, h);

  window.setWindowOpacity(EmojiPickerSettings::snapshot().windowOpacity());
  window.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  window.setWindowIcon(QIcon(":/res/72x72/1f0cf.png"));

  if (!EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    window.setStyleSheet(
        readQFileIfExists(QString::fromStdString(EmojiPickerSettings::snapshot().customQssFilePath())));
  }

  // if (EmojiPickerSettings::snapshot().openAtMouseLocation()) {
  //   int cursorX = 0;
  //   int cursorY = 0;
  //   crossdo_get_mouse_location2(crossdo.get(), &cursorX, &cursorY, nullptr, nullptr);

  //   if (cursorX != 0 && cursorY != 0) {
  //     window.move(cursorX, cursorY);
  //   }
  // }

  QMimeData* prevClipboardMimeData = nullptr;
  EmojiPicker* mainWidget = new EmojiPicker();
  mainWidget->setInputMethod(inputMethod);

  QObject::connect(mainWidget, &EmojiPicker::returnPressed, [&](const std::string& emojiStr, bool closeAfter) {
    wm::WId currentWindow = 0;

    if (activateWindowBeforeWriting || closeAfter) {
      currentWindow = wm::activeWindow();
    }

    if (currentWindow) {
      wm::activate(prevWindow);
    }

    if (useClipboardHack) {
      if (prevClipboardMimeData == nullptr) {
        prevClipboardMimeData = copyQMimeData(QApplication::clipboard()->mimeData());
      }

      QApplication::clipboard()->clear();
      QApplication::clipboard()->setText(QString::fromStdString(emojiStr));

      if (closeAfter) {
        if (uinputFile) {
          uinputFile.writeInputEvent(EV_KEY, KEY_LEFTSHIFT, uinput::KEY_RELEASE);
        } else {
          wm::clearModifiers(prevWindow);
        }
      }

      if (uinputFile) {
        uinputFile.writeKeypress(KEY_V, KEY_LEFTCTRL, SYN_REPORT);
      } else {
        wm::sendKeysequence(prevWindow, "ctrl+v");
      }
    } else {
      wm::enterText(prevWindow, emojiStr.data());
    }

    if (currentWindow && !closeAfter) {
      wm::activate(currentWindow);
    }
  });

  QObject::connect(mainWidget, &EmojiPicker::escapePressed, [&]() {
    window.hide();

    EmojiPickerSettings::writeDefaultsToDisk();

    if (useClipboardHack && prevClipboardMimeData != nullptr) {
      QTimer::singleShot(100, [&]() {
        QApplication::clipboard()->clear();
        QApplication::clipboard()->setMimeData(prevClipboardMimeData);

        if (!runAsDaemon) {
          QObject::connect(QApplication::clipboard(), &QClipboard::dataChanged, [&]() {
            app.exit();
          });
        }
      });
    } else {
      if (!runAsDaemon) {
        app.exit();
      }
    }
  });

  QObject::connect(mainWidget, &EmojiPicker::toggleInputMethod, [&]() {
    if (prevWindowProcessName.length() == 0) {
      return;
    }

    EmojiPickerSettings settings;
    settings.toggleInputMethod(prevWindowProcessName);

    activateWindowBeforeWriting = settings.activateWindowBeforeWriting(prevWindowProcessName);
    useClipboardHack = settings.useClipboardHack(prevWindowProcessName);

    mainWidget->setInputMethod(getInputMethod(prevWindowProcessName, settings));
  });

  window.setCentralWidget(mainWidget);

  if (runAsDaemon) {
    auto dbus = new EmojiPickerDBusInterface(&window, !!uinputFile ? QDBusConnection::SystemBus : QDBusConnection::SessionBus);
    dbus->_show = [&]() {
      prevWindow = wm::activeWindow();
      prevWindowPID = wm::pid(prevWindow);
      initState();
      mainWidget->setInputMethod(inputMethod);

      prevClipboardMimeData = nullptr;

      mainWidget->reset();

      window.show();

      wm::activate(window.winId());
    };
    dbus->_hide = [&]() {
      window.hide();
    };
  } else {
    window.show();
  }

  return app.exec();
}
