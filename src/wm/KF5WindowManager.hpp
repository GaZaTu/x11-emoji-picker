#pragma once

#include "../WindowManager.hpp"
#include <dlfcn.h>
#include <memory>
#include <unistd.h>

class libKF5WindowSystem {
public:
  using WId = unsigned long;

  using KWindowSystem_activeWindow_t = WId (*)();
  using KWindowSystem_activateWindow_t = void (*)(WId win, long time);

  KWindowSystem_activeWindow_t _activeWindow;
  KWindowSystem_activateWindow_t _activateWindow;

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

    _activeWindow = (KWindowSystem_activeWindow_t)dlsym(_ptr.get(), "_ZN13KWindowSystem12activeWindowEv");
    _activateWindow = (KWindowSystem_activateWindow_t)dlsym(_ptr.get(), "_ZN13KWindowSystem17forceActiveWindowEyl");
  }
};

class KF5WindowManager : public wm::WindowManager {
public:
  std::string name() override {
    return "KF5WindowManager";
  }

  operator bool() override {
    return !!_lib;
  }

  wm::WId activeWindow() override {
    return _lib._activeWindow();
  }

  void activate(wm::WId window) override {
    _lib._activateWindow(window, 0);
    usleep(20000);
  }

private:
  libKF5WindowSystem _lib;
};
