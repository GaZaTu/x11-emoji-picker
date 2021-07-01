#include "app.hpp"
#include "EmojiPicker.hpp"
#include "EmojiPickerSettings.hpp"
#include "WindowManager.hpp"
#include "uinput.hpp"
#include <QClipboard>
#include <QCommandLineParser>
#include <QMimeData>
#include <QTextStream>
#include <QTimer>

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

//   QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.PolicyKit1",
//   "/org/freedesktop/PolicyKit1/Authority", "org.freedesktop.PolicyKit1.Authority", "CheckAuthorization"); message <<
//   QVariant::fromValue(subject) << QVariant::fromValue(action_id) << QVariant::fromValue(details) <<
//   QVariant::fromValue(flags) << QVariant::fromValue(cancellation_id); QDBusMessage result =
//   QDBusConnection::systemBus().call(message);

//   qDebug() << result;
// }

app::args::args(QCoreApplication& app) {
  QCommandLineParser cliParser;
  cliParser.setApplicationDescription(PROJECT_NAME);
  cliParser.addHelpOption();
  cliParser.addVersionOption();

  QCommandLineOption daemonOption{"daemon",
      "run as dbus daemon (service: xyz.gazatu.EmojiPicker, object: /xyz/gazatu/EmojiPicker, methods: show & hide)"};
  cliParser.addOption(daemonOption);

  // QCommandLineOption dbusOption{"dbus", "bus to provide service on (default: $DBUS_SESSION_BUS_ADDRESS)"};
  // cliParser.addOption(dbusOption);

  cliParser.process(app);

  runAsDaemon = cliParser.isSet(daemonOption);
}

void app::main::initState() {
  if (!wm::supportsInput()) {
    uinputFd = uinput::open(PROJECT_NAME);
    if (uinputFd < 1) {
      uinputFd = 0;
    }
  }

  if (!uinputFd && !wm::supportsInput()) {
    prevWindowProcessName = "";
    activateWindowBeforeWriting = false;
    useClipboardHack = false;
    inputMethod = "none";
  } else if (uinputFd) {
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
}

app::main::main(QApplication& a, args& args) : app(a) {
  // qDebug() << "windowManager:" << wm::WindowManager::instance()->name().data();

  initState();

  if (!EmojiPickerSettings::snapshot().useSystemQtTheme()) {
    app.setStyle("fusion");
    app.setStyleSheet(readQFileIfExists(":/main.qss"));
  }

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

  window.setFixedSize(w, h);

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

  mainWidget = new EmojiPicker();
  mainWidget->setInputMethod(inputMethod);

  QObject::connect(mainWidget, &EmojiPicker::returnPressed, [this](const std::string& emojiStr, bool closeAfter) {
    wm::WId currentWindow = 0;

    if (activateWindowBeforeWriting || closeAfter) {
      currentWindow = wm::getWIdForQWindow(window);
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
        if (uinputFd) {
          uinput::writeInputEvent(uinputFd, EV_KEY, KEY_LEFTSHIFT, uinput::KEY_RELEASE);
        } else {
          wm::clearModifiers(prevWindow);
        }
      }

      if (uinputFd) {
        uinput::writeKeypress(uinputFd, KEY_V, KEY_LEFTCTRL, SYN_REPORT);
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

  QObject::connect(mainWidget, &EmojiPicker::escapePressed, [this, &args]() {
    window.hide();

    EmojiPickerSettings::writeDefaultsToDisk();

    if (useClipboardHack && prevClipboardMimeData != nullptr) {
      QTimer::singleShot(100, [&]() {
        QApplication::clipboard()->clear();
        QApplication::clipboard()->setMimeData(prevClipboardMimeData);

        if (!args.runAsDaemon) {
          QObject::connect(QApplication::clipboard(), &QClipboard::dataChanged, [&]() {
            app.exit();
          });
        }
      });
    } else {
      if (!args.runAsDaemon) {
        app.exit();
      }
    }
  });

  QObject::connect(mainWidget, &EmojiPicker::toggleInputMethod, [this]() {
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
}

void app::main::show() {
  prevWindow = wm::activeWindow();
  prevWindowPID = wm::pid(prevWindow);
  initState();
  mainWidget->setInputMethod(inputMethod);

  prevClipboardMimeData = nullptr;

  mainWidget->reset();

  window.show();

  wm::activate(wm::getWIdForQWindow(window));
}

void app::main::hide() {
  window.hide();
}
