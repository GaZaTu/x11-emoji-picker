#include "EmojiPickerDBusInterface.hpp"
#include "EmojiPickerDBusAdaptor.h"

// dbus-send --system --type=method_call --dest=xyz.gazatu.EmojiPicker /xyz/gazatu/EmojiPicker xyz.gazatu.EmojiPicker.show string:$DISPLAY string:$XAUTHORITY dict:string:string:DISPLAY,$DISPLAY,XAUTHORITY,$XAUTHORITY
// dbus-send --system --type=method_call --dest=xyz.gazatu.EmojiPicker /xyz/gazatu/EmojiPicker xyz.gazatu.EmojiPicker.show dict:string:string:WAYLAND_DISPLAY,$WAYLAND_DISPLAY

EmojiPickerDBusInterface::EmojiPickerDBusInterface(QObject* parent, QDBusConnection::BusType busType)
    : QObject(parent) {
  new EmojiPickerDBusAdaptor(this);

  QDBusConnection dbus =
      busType == QDBusConnection::SystemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus();

  if (!dbus.registerObject("/xyz/gazatu/EmojiPicker", this)) {
    throw std::runtime_error{"failed to register dbus object"};
  }

  if (!dbus.registerService("xyz.gazatu.EmojiPicker")) {
    throw std::runtime_error{"failed to register dbus service"};
  }
}

EmojiPickerDBusInterface::~EmojiPickerDBusInterface() {
}

void EmojiPickerDBusInterface::show(const QMap<QString, QString>& env) {
  _show(env);
}

void EmojiPickerDBusInterface::hide() {
  _hide();
}
