#pragma once

#include <memory>
#include <string>

namespace wm {
using WId = uint64_t;

class WindowManager {
public:
  static std::shared_ptr<WindowManager> instance();

  virtual ~WindowManager() {
  }

  virtual std::string name() = 0;

  virtual operator bool() = 0;

  virtual WId activeWindow() = 0;

  virtual int pid(WId window) {
    return 0;
  }

  virtual void activate(WId window) = 0;

  virtual void clearModifiers(WId window) {
  }

  virtual void sendKeysequence(WId window, const char* sequence) {
  }

  virtual void enterText(WId window, const char* text) {
  }

  virtual bool supportsInput() {
    return false;
  }
};

WId activeWindow() {
  return WindowManager::instance()->activeWindow();
}

int pid(WId window) {
  return WindowManager::instance()->pid(window);
}

void activate(WId window) {
  WindowManager::instance()->activate(window);
}

void clearModifiers(WId window) {
  WindowManager::instance()->clearModifiers(window);
}

void sendKeysequence(WId window, const char* sequence) {
  WindowManager::instance()->sendKeysequence(window, sequence);
}

void enterText(WId window, const char* text) {
  WindowManager::instance()->enterText(window, text);
}

bool supportsInput() {
  return WindowManager::instance()->supportsInput();
}
} // namespace wm
