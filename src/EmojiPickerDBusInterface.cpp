#include "EmojiPickerDBusInterface.hpp"
#include "EmojiPickerDBusAdaptor.h"

// dbus-send --system --type=method_call --dest=xyz.gazatu.EmojiPicker /xyz/gazatu/EmojiPicker xyz.gazatu.EmojiPicker.show string:$DISPLAY string:$XAUTHORITY
// dbus-send --system --type=method_call --dest=xyz.gazatu.EmojiPicker /xyz/gazatu/EmojiPicker xyz.gazatu.EmojiPicker.show string:$WAYLAND_DISPLAY string:'!wayland'

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

void EmojiPickerDBusInterface::show(const QString& display, const QString& xauthority) {
  _show(display, xauthority);
}

void EmojiPickerDBusInterface::hide() {
  _hide();
}
