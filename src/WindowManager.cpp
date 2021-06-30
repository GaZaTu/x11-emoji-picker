#include "WindowManager.hpp"
#include "wm/KF5WindowManager.hpp"
#include "wm/SwayWindowManager.hpp"
#include "wm/XDoWindowManager.hpp"

std::shared_ptr<wm::WindowManager> _instance;
std::shared_ptr<wm::WindowManager> wm::WindowManager::instance() {
  if (!_instance) {
    _instance = std::make_shared<XDoWindowManager>();
    if (*_instance) {
      return _instance;
    }

    _instance = std::make_shared<SwayWindowManager>();
    if (*_instance) {
      return _instance;
    }

    _instance = std::make_shared<KF5WindowManager>();
    if (*_instance) {
      return _instance;
    }
  }

  return _instance;
}

wm::WId wm::activeWindow() {
  return wm::WindowManager::instance()->activeWindow();
}

int wm::pid(wm::WId window) {
  return wm::WindowManager::instance()->pid(window);
}

void wm::activate(wm::WId window) {
  wm::WindowManager::instance()->activate(window);
}

void wm::clearModifiers(wm::WId window) {
  wm::WindowManager::instance()->clearModifiers(window);
}

void wm::sendKeysequence(wm::WId window, const char* sequence) {
  wm::WindowManager::instance()->sendKeysequence(window, sequence);
}

void wm::enterText(wm::WId window, const char* text) {
  wm::WindowManager::instance()->enterText(window, text);
}

bool wm::supportsInput() {
  return wm::WindowManager::instance()->supportsInput();
}
