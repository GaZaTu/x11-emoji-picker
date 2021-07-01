#include "EmojiPickerDBusInterface.hpp"
#include "EmojiPickerSettings.hpp"
#include "EmojiTranslator.hpp"
#include "app.hpp"
#include <QtDBus>
#include <unistd.h>

constexpr int UPGRADE_TO_QAPPLICATION = -123;

int main(int argc, char** argv) {
  QCoreApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QCoreApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QCoreApplication::setApplicationName(PROJECT_NAME);
  QCoreApplication::setApplicationVersion(PROJECT_VERSION);

  int rc = 0;
  std::shared_ptr<QCoreApplication> coreApp = std::make_shared<QCoreApplication>(argc, argv);
  app::args args{*coreApp};
  std::shared_ptr<QApplication> guiApp;
  std::shared_ptr<app::main> guiMain;

  EmojiPickerDBusInterface* dbus = nullptr;

  QDBusConnection::BusType dbusBusType = QDBusConnection::SessionBus;
  if (getuid() == 0) {
    dbusBusType = QDBusConnection::SystemBus;
  }

  auto upgradeToQApplication = [&]() {
    coreApp->exit(UPGRADE_TO_QAPPLICATION);
    rc = UPGRADE_TO_QAPPLICATION;
  };

  if (args.runAsDaemon) {
    EmojiPickerSettings::snapshotScope = QSettings::SystemScope;

    dbus = new EmojiPickerDBusInterface(coreApp.get(), dbusBusType);
    dbus->_show = [&](const QMap<QString, QString>& env) {
      for (auto& key : env) {
        setenv(key.toStdString().data(), env[key].toStdString().data(), true);
      }

      upgradeToQApplication();
    };
  } else {
    upgradeToQApplication();
  }

  if (rc != UPGRADE_TO_QAPPLICATION) {
    rc = coreApp->exec();
  }

  if (rc == UPGRADE_TO_QAPPLICATION) {
    coreApp = nullptr;

    guiApp = std::make_shared<QApplication>(argc, argv);
    guiMain = std::make_shared<app::main>(*guiApp, args);

    if (args.runAsDaemon) {
      dbus = new EmojiPickerDBusInterface(guiApp.get(), dbusBusType);
      dbus->_show = [&](const QMap<QString, QString>&) {
        guiMain->show();
      };
      dbus->_hide = [&]() {
        guiMain->hide();
      };
    }

    auto translator = new EmojiTranslator(guiApp.get(), EmojiPickerSettings::snapshot().localeKey());
    QCoreApplication::installTranslator(translator);

    guiMain->show();
    rc = guiApp->exec();
  }

  return rc;
}
