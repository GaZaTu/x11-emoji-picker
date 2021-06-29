#include "WindowManager.hpp"
#include <dlfcn.h>

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
  using xdo_clear_active_modifiers_t = int (*)(const xdo_t* xdo, Window window, charcodemap_t* active_mods, int active_mods_n);
  using xdo_send_keysequence_window_t = int (*)(const xdo_t* xdo, Window window, const char* keysequence, useconds_t delay);
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

class XDoWindow : public Window {
public:
  XDoWindow(libxdo lib, libxdo::xdo_t* xdo, libxdo::Window window) : _lib(lib), _xdo(xdo), _window(window) {};

  int pid() override {
    return _lib._get_pid_window(_xdo, _window);
  }

  void activate() override {
    _lib._activate_window(_xdo, _window);
    _lib._wait_for_window_active(_xdo, _window, 1);
  }

  void clearModifiers() override {
    libxdo::charcodemap_t* keys;
    int keysLen;

    _lib._get_active_modifiers(_xdo, &keys, &keysLen);
    _lib._clear_active_modifiers(_xdo, _window, keys, keysLen);
  }

  void sendKeysequence(const char* sequence) override {
    _lib._send_keysequence_window(_xdo, _window, sequence, 12000);
  }

  void enterText(const char* text) override {
    _lib._enter_text_window(_xdo, _window, text, 12000);
  }

private:
  libxdo _lib;
  libxdo::xdo_t* _xdo;
  libxdo::Window _window;
};

class XDoWindowManager : public WindowManager {
public:
  XDoWindowManager() {
    if (*this) {
      _xdo = _lib._new(nullptr);
    }
  }

  virtual ~XDoWindowManager() {
    _lib._free(_xdo);
  }

  std::string name() override {
    return "XDoWindowManager";
  }

  operator bool() override {
    return !!_lib;
  }

  std::shared_ptr<Window> activeWindow() override {
    libxdo::Window window;
    _lib._get_active_window(_xdo, &window);

    return std::make_shared<XDoWindow>(_lib, _xdo, window);
  }

  bool supportsInput() override {
    return true;
  }

private:
  libxdo _lib;
  libxdo::xdo_t* _xdo;
};

class libKF5WindowSystem {
public:
  using WId = unsigned long;

  using KF5WindowSystem_activeWindow_t = WId (*)();
  // using xdo_get_pid_window_t = int (*)(const xdo_t* xdo, Window window);
  using KF5WindowSystem_activateWindow_t = void (*)(WId win, long time);
  // using xdo_wait_for_window_active_t = int (*)(const xdo_t* xdo, Window window, int active);
  // using xdo_get_active_modifiers_t = int (*)(const xdo_t* xdo, charcodemap_t** keys, int* nkeys);
  // using xdo_clear_active_modifiers_t = int (*)(const xdo_t* xdo, Window window, charcodemap_t* active_mods, int active_mods_n);
  // using xdo_send_keysequence_window_t = int (*)(const xdo_t* xdo, Window window, const char* keysequence, useconds_t delay);
  // using xdo_enter_text_window_t = int (*)(const xdo_t* xdo, Window window, const char* string, useconds_t delay);

  KF5WindowSystem_activeWindow_t _activeWindow;
  // xdo_get_pid_window_t _get_pid_window;
  KF5WindowSystem_activateWindow_t _activateWindow;
  // xdo_wait_for_window_active_t _wait_for_window_active;
  // xdo_get_active_modifiers_t _get_active_modifiers;
  // xdo_clear_active_modifiers_t _clear_active_modifiers;
  // xdo_send_keysequence_window_t _send_keysequence_window;
  // xdo_enter_text_window_t _enter_text_window;

  libKF5WindowSystem(const std::string& file = "libKF5WindowSystem.so.5") {
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

    _activeWindow = (KF5WindowSystem_activeWindow_t)dlsym(_ptr.get(), "_ZN13KWindowSystem12activeWindowEv");
    // _get_pid_window = (xdo_get_pid_window_t)dlsym(_ptr.get(), "xdo_get_pid_window");
    _activateWindow = (KF5WindowSystem_activateWindow_t)dlsym(_ptr.get(), "_ZN13KWindowSystem14activateWindowEyl");
    // _wait_for_window_active = (xdo_wait_for_window_active_t)dlsym(_ptr.get(), "xdo_wait_for_window_active");
    // _get_active_modifiers = (xdo_get_active_modifiers_t)dlsym(_ptr.get(), "xdo_get_active_modifiers");
    // _clear_active_modifiers = (xdo_clear_active_modifiers_t)dlsym(_ptr.get(), "xdo_clear_active_modifiers");
    // _send_keysequence_window = (xdo_send_keysequence_window_t)dlsym(_ptr.get(), "xdo_send_keysequence_window");
    // _enter_text_window = (xdo_enter_text_window_t)dlsym(_ptr.get(), "xdo_enter_text_window");
  }
};

class KF5Window : public Window {
public:
  KF5Window(libKF5WindowSystem lib, libKF5WindowSystem::WId window) : _lib(lib), _window(window) {};

  void activate() override {
    _lib._activateWindow(_window, 0);
  }

private:
  libKF5WindowSystem _lib;
  libKF5WindowSystem::WId _window;
};

class KF5WindowManager : public WindowManager {
public:
  std::string name() override {
    return "KF5WindowManager";
  }

  operator bool() override {
    return !!_lib;
  }

  std::shared_ptr<Window> activeWindow() override {
    return std::make_shared<KF5Window>(_lib, _lib._activeWindow());
  }

private:
  libKF5WindowSystem _lib;
};

std::shared_ptr<WindowManager> _instance;
std::shared_ptr<WindowManager> WindowManager::instance() {
  if (!_instance) {
    _instance = std::make_shared<XDoWindowManager>();
    if (*_instance) {
      return _instance;
    }

    // _instance = std::make_shared<KF5WindowManager>();
    // if (*_instance) {
    //   return _instance;
    // }
  }

  return _instance;
}
