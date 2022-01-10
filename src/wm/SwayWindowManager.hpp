#pragma once

#include "../WindowManager.hpp"
#include <QCoreApplication>
#include <QDataStream>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
// #include <QLocalSocket>
#include <QProcess>
#include <memory>
#include <unistd.h>
#include <QDebug>
#include <set>

#ifdef WAYLAND_PROTOCOLS
#include "wayland-input-method-unstable-v1-client-protocol.h"
#include "wayland-input-method-unstable-v2-client-protocol.h"

namespace wl {
std::shared_ptr<wl_display> display(const char* name = nullptr) {
  auto ptr = wl_display_connect(name);
  if (!ptr) {
    return nullptr;
  }

  return {ptr, &wl_display_disconnect};
}

std::shared_ptr<wl_registry> registry(std::shared_ptr<wl_display> display) {
  auto ptr = wl_display_get_registry(&*display);
  if (!ptr) {
    return nullptr;
  }

  return {ptr, &wl_registry_destroy};
}

using registry_listener_func = std::function<void(wl_registry*, uint32_t, const char*, uint32_t)>;
wl_registry_listener createRegistryListener() {
  wl_registry_listener listener;
  listener.global = [](void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    (*(registry_listener_func*)data)(registry, name, interface, version);
  };
  listener.global_remove = [](void* data, wl_registry* registry, uint32_t name) {
  };

  return listener;
}

class input_integration {
public:
  std::shared_ptr<wl_display> _display;
  std::shared_ptr<wl_registry> _registry;

  wl_registry_listener _registry_listener;
  wl::registry_listener_func _registry_listener_func;

  std::set<std::string> _ignored_interfaces;

  wl_seat* _active_seat = nullptr;

#ifdef ZWP_TEXT_INPUT_V3_INTERFACE
  zwp_text_input_manager_v3* _text_input_manager_v3 = nullptr;
#endif

#ifdef ZWP_INPUT_METHOD_V2_INTERFACE
  zwp_input_method_manager_v2* _input_method_manager_v2 = nullptr;
#endif

  input_integration() {
    _display = wl::display();
    _registry = wl::registry(_display);

    _registry_listener = wl::createRegistryListener();
    _registry_listener_func = [this](wl_registry* registry, uint32_t name, const char* _interface, uint32_t version) {
      qDebug() << "interface" << _interface;

      std::string interface{_interface};

      if (_ignored_interfaces.count(interface) == 1) {
        return;
      }

      if (interface == wl_seat_interface.name) {
        _active_seat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, version <= 7 ? version : 7);
      }

#ifdef ZWP_TEXT_INPUT_V3_INTERFACE
      else if (interface == zwp_text_input_manager_v3_interface.name) {
        _text_input_manager_v3 =
            (zwp_text_input_manager_v3*)wl_registry_bind(registry, name, &zwp_text_input_manager_v3_interface, 1);
      }
#endif

#ifdef ZWP_INPUT_METHOD_V2_INTERFACE
      else if (interface == zwp_input_method_manager_v2_interface.name) {
        _input_method_manager_v2 =
            (zwp_input_method_manager_v2*)wl_registry_bind(registry, name, &zwp_input_method_manager_v2_interface, 1);
      }
#endif
    };

    wl_registry_add_listener(&*_registry, &_registry_listener, &_registry_listener_func);

    roundtrip();
  }

  int capabilities() {
    return 0; // active_window | activate_window | commit_string
  }

  void ignore(const wl_interface& interface) {
    _ignored_interfaces.emplace(interface.name);
  }

  void roundtrip() {
    wl_display_dispatch(&*_display);
    wl_display_roundtrip(&*_display);
  }

  void commit_string(const char* text) {
    if (!_active_seat) {
      return;
    }

#ifdef ZWP_TEXT_INPUT_V3_INTERFACE
    if (_text_input_manager_v3) {
      text_input_v3_commit_string(text);
      return;
    }
#endif

#ifdef ZWP_INPUT_METHOD_V2_INTERFACE
    if (_input_method_manager_v2) {
      input_method_v2_commit_string(text);
      return;
    }
#endif

    throw std::runtime_error{"wayland compositor does not support any known input method protocol"};
  }

#ifdef ZWP_TEXT_INPUT_V3_INTERFACE
  std::shared_ptr<zwp_text_input_v3> get_text_input_v3() {
    auto ptr = zwp_text_input_manager_v3_get_text_input(_text_input_manager_v3, _active_seat);
    if (!ptr) {
      return nullptr;
    }

    return {ptr, &zwp_text_input_v3_destroy};
  }

  void text_input_v3_commit_string(const char* text) {
    auto text_input_v3 = get_text_input_v3();

    zwp_text_input_v3_listener listener;
    listener.enter = [](auto, auto, auto) {
      qDebug() << "listener.enter";
    };
    listener.leave = [](auto, auto, auto) {
      qDebug() << "listener.leave";
    };
    listener.preedit_string = [](auto, auto, auto, auto, auto) {
      qDebug() << "listener.preedit_string";
    };
    listener.commit_string = [](auto, auto, auto) {
      qDebug() << "listener.commit_string";
    };
    listener.delete_surrounding_text = [](auto, auto, auto, auto) {
      qDebug() << "listener.delete_surrounding_text";
    };
    listener.done = [](auto, auto, auto) {
      qDebug() << "listener.done";
    };

    zwp_text_input_v3_add_listener(&*text_input_v3, &listener, nullptr);

    roundtrip();

    // zwp_text_input_v3_set_surrounding_text(&*text_input_v3, text, 0, 0);
    // zwp_text_input_v3_commit(&*text_input_v3);

    // roundtrip();
  }
#endif

#ifdef ZWP_INPUT_METHOD_V2_INTERFACE
  std::shared_ptr<zwp_input_method_v2> get_input_method_v2() {
    auto ptr = zwp_input_method_manager_v2_get_input_method(_input_method_manager_v2, _active_seat);
    if (!ptr) {
      return nullptr;
    }

    return {ptr, &zwp_input_method_v2_destroy};
  }

  void input_method_v2_commit_string(const char* text) {
    auto input_method_v2 = get_input_method_v2();

    // zwp_input_method_v2_listener listener;
    // listener.activate = [](auto, auto) {
    //   qDebug() << "listener.activate";
    // };
    // listener.deactivate = [](auto, auto) {
    //   qDebug() << "listener.deactivate";
    // };
    // listener.surrounding_text = [](void *data, zwp_input_method_v2 *zwp_input_method_v2, const char *text, uint32_t cursor, uint32_t anchor) {
    //   qDebug() << "listener.surrounding_text" << text << cursor << anchor;
    // };
    // listener.text_change_cause = [](auto, auto, auto) {
    //   qDebug() << "listener.text_change_cause";
    // };
    // listener.content_type = [](auto, auto, auto, auto) {
    //   qDebug() << "listener.content_type";
    // };
    // listener.done = [](auto, auto) {
    //   qDebug() << "listener.done";
    // };
    // listener.unavailable = [](auto, auto) {
    //   qDebug() << "listener.unavailable";
    // };

    // zwp_input_method_v2_add_listener(&*input_method_v2, &listener, nullptr);

    roundtrip();

    zwp_input_method_v2_commit_string(&*input_method_v2, text);
    zwp_input_method_v2_commit(&*input_method_v2, 0);

    roundtrip();
  }
#endif
};
} // namespace wl
#endif

namespace sway {
static constexpr char SWAYSOCK[] = "SWAYSOCK";
static constexpr char MAGIC_STRING[] = "i3-ipc";

static constexpr int RUN_COMMAND = 0;
static constexpr int GET_TREE = 4;
} // namespace sway

class SwayWindowManager : public wm::WindowManager {
public:
  SwayWindowManager() {
    const char* swaysock = getenv(sway::SWAYSOCK);
    if (swaysock == nullptr) {
      return;
    }

    // _socket.connectToServer(swaysock);
    // _socket.waitForConnected();

    _valid = true;
  }

  std::string name() override {
    return "SwayWindowManager";
  }

  operator bool() override {
    return _valid;
  }

  wm::WId activeWindow() override {
    // QString payload = "";
    // QByteArray input = createMessage(GET_TREE, payload);

    // _socket.write(input);
    // _socket.waitForBytesWritten();
    // _socket.waitForReadyRead();

    QProcess process;
    process.setProgram("swaymsg");
    process.setArguments({"-t", "get_tree"});
    process.start();
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QJsonDocument document = QJsonDocument::fromJson(output);

    // QByteArray output = _socket.readAll();
    // QJsonDocument document = parseReply(output);
    QJsonObject root = document.object();

    QJsonObject activeWindow = getFocusedNodeInTree(root);
    if (activeWindow.isEmpty()) {
      return 0;
    }

    return activeWindow["id"].toInt();
  }

  void activate(wm::WId window) override {
    QString payload = QString{"[con_id=%1] focus"}.arg(window);
    if (window == -1) {
      payload = QString{"[pid=%1] focus"}.arg(QCoreApplication::applicationPid());
    }

    // QByteArray input = createMessage(RUN_COMMAND, payload);

    // _socket.write(input);
    // _socket.waitForBytesWritten();
    // _socket.waitForReadyRead();

    // _socket.readAll();

    QProcess process;
    process.setProgram("swaymsg");
    process.setArguments({payload});
    process.start();
    process.waitForFinished();
  }

  wm::WId getWIdForQWindow(const QWidget& window) override {
    return -1;
  }

#ifdef WAYLAND_PROTOCOLS
  void enterText(wm::WId window, const char* text) override {
    wl::input_integration wlinput;
    wlinput.commit_string(text);
  }
#endif

  int capabilities() override {
#ifdef WAYLAND_PROTOCOLS
    return wm::SUPPORTS_TEXT_INPUT;
#else
    return 0;
#endif
  }

private:
  // QLocalSocket _socket;
  bool _valid = false;

  // static QByteArray createMessage(int type, const QString& payload) {
  //   QByteArray input;
  //   QDataStream istream{&input, QIODevice::WriteOnly};
  //   istream << MAGIC_STRING;
  //   istream << (int32_t)payload.length();
  //   istream << (int32_t)type;
  //   istream << payload;

  //   return input;
  // }

  // static QJsonDocument parseReply(QByteArray& output) {
  //   output.remove(0, sizeof(MAGIC_STRING));
  //   output.remove(0, sizeof(int32_t));
  //   output.remove(0, sizeof(int32_t));

  //   return QJsonDocument::fromJson(output);
  // }

  static QJsonObject getFocusedNodeInTree(QJsonObject& node) {
    if (node["focused"].toBool() == true) {
      return node;
    }

    for (QJsonValue child : node["nodes"].toArray()) {
      QJsonObject childObject = child.toObject();
      QJsonObject focusedChild = getFocusedNodeInTree(childObject);

      if (!focusedChild.isEmpty()) {
        return focusedChild;
      }
    }

    return {};
  }
};
