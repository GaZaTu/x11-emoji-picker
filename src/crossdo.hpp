#pragma once

#ifdef __linux__
extern "C" {
#include "xdo.h"
}
#elif _WIN32
#include "windows.h"
#endif

#ifdef __linux__
typedef xdo_t crossdo_t;
#elif _WIN32
typedef struct {
} crossdo_t;
#endif

#ifdef __linux__
typedef Window window_t;
#elif _WIN32
typedef HWND window_t;
#endif

crossdo_t* crossdo_new() {
#ifdef __linux__
  return xdo_new(NULL);
#elif _WIN32
  return new crossdo_t();
#endif
}

void crossdo_free(crossdo_t* crossdo) {
#ifdef __linux__
  xdo_free(crossdo);
#elif _WIN32
  delete crossdo;
#endif
}

int crossdo_get_active_window(crossdo_t* crossdo, window_t* result) {
#ifdef __linux__
  return xdo_get_active_window(crossdo, result);
#elif _WIN32
  *result = GetActiveWindow();

  return 0;
#endif
}

int crossdo_enter_text_window(crossdo_t* crossdo, window_t window, const char* string, unsigned int delay) {
#ifdef __linux__
  return xdo_enter_text_window(crossdo, window, string, delay);
#elif _WIN32
  SendMessage(window, WM_SETTEXT, NULL, (LPARAM)string);

  return 0;
#endif
}
