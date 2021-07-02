#pragma once

#include <QWidget>
#include <memory>
#include <string>

namespace wm {
using WId = uint64_t;

constexpr int SUPPORTS_KEYSEQUENCE_INPUT = 1 << 0;
constexpr int SUPPORTS_TEXT_INPUT = 1 << 1;
constexpr int SUPPORTS_UNFOCUSED_INPUT = 1 << 2;

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

  virtual WId getWIdForQWindow(const QWidget& window) {
    return window.winId();
  }

  virtual int capabilities() {
    return 0;
  }
};

WId activeWindow();

int pid(WId window);

void activate(WId window);

void clearModifiers(WId window);

void sendKeysequence(WId window, const char* sequence);

void enterText(WId window, const char* text);

WId getWIdForQWindow(const QWidget& window);

int capabilities();
} // namespace wm
