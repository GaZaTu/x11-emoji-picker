#include "EmojiPickerDBusInterface.hpp"
#include "EmojiPickerDBusAdaptor.h"

// dbus-send --system --type=method_call --dest=xyz.gazatu.EmojiPicker /xyz/gazatu/EmojiPicker xyz.gazatu.EmojiPicker.show

EmojiPickerDBusInterface::EmojiPickerDBusInterface(QObject* parent, QDBusConnection::BusType busType)
    : QObject(parent) {
  new EmojiPickerDBusAdaptor(this);

  QDBusConnection dbus =
      busType == QDBusConnection::SystemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus();
  // QDBusConnection dbus = QDBusConnection::connectToBus("unix:path=/run/user/77000927/bus", "emojipicker");
  // QDBusConnection dbus = QDBusConnection::sessionBus();

  if (!dbus.registerObject("/xyz/gazatu/EmojiPicker", this)) {
    throw std::runtime_error{"failed to register dbus object"};
  }

  if (!dbus.registerService("xyz.gazatu.EmojiPicker")) {
    throw std::runtime_error{"failed to register dbus service"};
  }
}

EmojiPickerDBusInterface::~EmojiPickerDBusInterface() {
}

void EmojiPickerDBusInterface::show() {
  _show();
}

void EmojiPickerDBusInterface::hide() {
  _hide();
}
