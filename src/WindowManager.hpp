#pragma once

#include <memory>
#include <string>

class Window {
public:
  virtual ~Window() {}

  virtual int pid() {
    return 0;
  }

  virtual void activate() = 0;

  virtual void clearModifiers() {}

  virtual void sendKeysequence(const char* sequence) {}

  virtual void enterText(const char* text) {}
};

class WindowManager {
public:
  static std::shared_ptr<WindowManager> instance();

  virtual ~WindowManager() {}

  virtual std::string name() = 0;

  virtual operator bool() = 0;

  virtual std::shared_ptr<Window> activeWindow() = 0;

  virtual bool supportsInput() {
    return false;
  }
};
