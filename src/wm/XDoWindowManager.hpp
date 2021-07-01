#pragma once

#include "../WindowManager.hpp"
#include <dlfcn.h>
#include <memory>
#include <string>
#include <unistd.h>

class libxdo {
public:
  using xdo_t = void;
  using charcodemap_t = void;
  using Window = unsigned long;

  using xdo_new_t = xdo_t* (*)(const char* display);
  using xdo_free_t = void (*)(xdo_t* xdo);
  using xdo_get_active_window_t = int (*)(const xdo_t* xdo, Window* window_ret);
  using xdo_get_pid_window_t = int (*)(const xdo_t* xdo, Window window);
  using xdo_activate_window_t = int (*)(const xdo_t* xdo, Window wid);
  using xdo_wait_for_window_active_t = int (*)(const xdo_t* xdo, Window window, int active);
  using xdo_get_active_modifiers_t = int (*)(const xdo_t* xdo, charcodemap_t** keys, int* nkeys);
  using xdo_clear_active_modifiers_t = int (*)(
      const xdo_t* xdo, Window window, charcodemap_t* active_mods, int active_mods_n);
  using xdo_send_keysequence_window_t = int (*)(
      const xdo_t* xdo, Window window, const char* keysequence, useconds_t delay);
  using xdo_enter_text_window_t = int (*)(const xdo_t* xdo, Window window, const char* string, useconds_t delay);

  xdo_new_t _new;
  xdo_free_t _free;
  xdo_get_active_window_t _get_active_window;
  xdo_get_pid_window_t _get_pid_window;
  xdo_activate_window_t _activate_window;
  xdo_wait_for_window_active_t _wait_for_window_active;
  xdo_get_active_modifiers_t _get_active_modifiers;
  xdo_clear_active_modifiers_t _clear_active_modifiers;
  xdo_send_keysequence_window_t _send_keysequence_window;
  xdo_enter_text_window_t _enter_text_window;

  libxdo(const std::string& file = "libxdo.so") {
    if (std::string{getenv("XDG_SESSION_TYPE")} != "x11") {
      return;
    }

    auto ptr = dlopen(file.data(), RTLD_LAZY);
    if (!ptr) {
      return;
    }

    _ptr = {ptr, &dlclose};

    loadFunctions();
  }

  operator bool() {
    return !!_ptr;
  }

private:
  std::shared_ptr<void> _ptr;

  void loadFunctions() {
    if (!_ptr) {
      return;
    }

    _new = (xdo_new_t)dlsym(_ptr.get(), "xdo_new");
    _free = (xdo_free_t)dlsym(_ptr.get(), "xdo_free");
    _get_active_window = (xdo_get_active_window_t)dlsym(_ptr.get(), "xdo_get_active_window");
    _get_pid_window = (xdo_get_pid_window_t)dlsym(_ptr.get(), "xdo_get_pid_window");
    _activate_window = (xdo_activate_window_t)dlsym(_ptr.get(), "xdo_activate_window");
    _wait_for_window_active = (xdo_wait_for_window_active_t)dlsym(_ptr.get(), "xdo_wait_for_window_active");
    _get_active_modifiers = (xdo_get_active_modifiers_t)dlsym(_ptr.get(), "xdo_get_active_modifiers");
    _clear_active_modifiers = (xdo_clear_active_modifiers_t)dlsym(_ptr.get(), "xdo_clear_active_modifiers");
    _send_keysequence_window = (xdo_send_keysequence_window_t)dlsym(_ptr.get(), "xdo_send_keysequence_window");
    _enter_text_window = (xdo_enter_text_window_t)dlsym(_ptr.get(), "xdo_enter_text_window");
  }
};

class XDoWindowManager : public wm::WindowManager {
public:
  XDoWindowManager() {
    if (*this) {
      _xdo = _lib._new(nullptr);
    }
  }

  virtual ~XDoWindowManager() {
    if (*this) {
      _lib._free(_xdo);
    }
  }

  std::string name() override {
    return "XDoWindowManager";
  }

  operator bool() override {
    return !!_lib;
  }

  wm::WId activeWindow() override {
    libxdo::Window window;
    _lib._get_active_window(_xdo, &window);

    return window;
  }

  int pid(wm::WId window) override {
    return _lib._get_pid_window(_xdo, window);
  }

  void activate(wm::WId window) override {
    _lib._activate_window(_xdo, window);
    _lib._wait_for_window_active(_xdo, window, 1);
  }

  void clearModifiers(wm::WId window) override {
    libxdo::charcodemap_t* keys;
    int keysLen;

    _lib._get_active_modifiers(_xdo, &keys, &keysLen);
    _lib._clear_active_modifiers(_xdo, window, keys, keysLen);
  }

  void sendKeysequence(wm::WId window, const char* sequence) override {
    _lib._send_keysequence_window(_xdo, window, sequence, 12000);
  }

  void enterText(wm::WId window, const char* text) override {
    _lib._enter_text_window(_xdo, window, text, 12000);
  }

  bool supportsInput() override {
    return true;
  }

private:
  libxdo _lib;
  libxdo::xdo_t* _xdo;
};
