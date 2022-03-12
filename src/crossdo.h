#pragma once

#ifdef __linux__
#ifdef __cplusplus
extern "C" {
#endif
#include "xdo.h"
#ifdef __cplusplus
}
#endif
#elif _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#define WIN32_LEAN_AND_MEAN 1
#define NOGDICAPMASKS 1
#define NOVIRTUALKEYCODES 1
// #define NOWINMESSAGES 1
#define NOWINSTYLES 1
#define NOSYSMETRICS 1
#define NOMENUS 1
#define NOICONS 1
#define NOKEYSTATES 1
#define NOSYSCOMMANDS 1
#define NORASTEROPS 1
#define NOSHOWWINDOW 1
#define OEMRESOURCE 1
#define NOATOM 1
// #define NOCLIPBOARD 1
#define NOCOLOR 1
#define NOCTLMGR 1
#define NODRAWTEXT 1
#define NOGDI 1
#define NOKERNEL 1
// #define NOUSER 1
// #define NONLS 1
#define NOMB 1
#define NOMEMMGR 1
#define NOMETAFILE 1
#define NOMINMAX 1
// #define NOMSG 1
#define NOOPENFILE 1
#define NOSCROLL 1
#define NOSERVICE 1
#define NOSOUND 1
#define NOTEXTMETRIC 1
#define NOWH 1
#define NOWINOFFSETS 1
#define NOCOMM 1
#define NOKANJI 1
#define NOHELP 1
#define NOPROFILER 1
#define NODEFERWINDOWPOS 1
#define NOMCX 1
#endif
// https://docs.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers
#include "windows.h"
#include "windowsx.h"
#include "ole2.h"
#include "oleacc.h"
#include "UIAutomation.h"
#include "stringapiset.h"
#endif

#ifdef __linux__
typedef Window window_t;
#elif _WIN32
typedef HWND window_t;
#endif

#ifdef __linux__
typedef xdo_t crossdo_t;
#elif _WIN32
typedef struct {
  IUIAutomation* ui_automation;
  IUIAutomationElement* ui_element;
} crossdo_t;
#endif

crossdo_t* crossdo_new() {
#ifdef __linux__
  return xdo_new(NULL);
#elif _WIN32
  if (FAILED(CoInitialize(NULL))) {
    printf("CoInitialize(...) failed");
    return NULL;
  }

  crossdo_t* crossdo = (crossdo_t*)malloc(sizeof(crossdo_t));
  crossdo->ui_automation = NULL;
  crossdo->ui_element = NULL;
  return crossdo;
#endif
}

void crossdo_free(crossdo_t* crossdo) {
#ifdef __linux__
  xdo_free(crossdo);
#elif _WIN32
  free(crossdo);

  CoUninitialize();
#endif
}

int crossdo_get_mouse_location2(
    crossdo_t* crossdo, int* x_ret, int* y_ret, int* screen_num_ret, window_t* window_ret) {
#ifdef __linux__
  return xdo_get_mouse_location2(crossdo, x_ret, y_ret, screen_num_ret, window_ret);
#elif _WIN32
  POINT cursor;
  GetCursorPos(&cursor);

  if (x_ret != NULL)
    *x_ret = cursor.x;

  if (y_ret != NULL)
    *y_ret = cursor.y;

  if (screen_num_ret != NULL)
    *screen_num_ret = 0;

  if (window_ret != NULL)
    *window_ret = WindowFromPoint(cursor);

  return 0;
#endif
}

int crossdo_get_caret_location2(
    crossdo_t* crossdo, int* x_ret, int* y_ret, int* screen_num_ret, window_t* window_ret) {
#ifdef __linux__
  *x_ret = 0;
  *y_ret = 0;
  *screen_num_ret = 0;
  *window_ret = 0;

  return 0;
#elif _WIN32
  *x_ret = 0;
  *y_ret = 0;
  *screen_num_ret = 0;
  *window_ret = 0;

  return 0;
#endif
}

int crossdo_get_pid_window(crossdo_t* crossdo, window_t window) {
#ifdef __linux__
  return xdo_get_pid_window(crossdo, window);
#elif _WIN32
  unsigned long pid = 0;
  GetWindowThreadProcessId(window, &pid);

  return pid;
#endif
}

int crossdo_get_active_window(crossdo_t* crossdo, window_t* window_ret) {
#ifdef __linux__
  return xdo_get_active_window(crossdo, window_ret);
#elif _WIN32
  *window_ret = GetForegroundWindow();

  if (crossdo->ui_element == NULL) {
    if (FAILED(CoCreateInstance(
            __uuidof(CUIAutomation8), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&crossdo->ui_automation)))) {
      printf("CoCreateInstance(__uuidof(CUIAutomation8), ...) failed");
      return -1;
    }

    if (FAILED(crossdo->ui_automation->GetFocusedElement(&crossdo->ui_element))) {
      printf("ui_automation->GetFocusedElement(...) failed");
      return -1;
    }
  }

  return 0;
#endif
}

int crossdo_activate_window(crossdo_t* crossdo, window_t window) {
#ifdef __linux__
  return xdo_activate_window(crossdo, window);
#elif _WIN32
  return 0;
#endif
}

int crossdo_wait_for_window_active(crossdo_t* crossdo, window_t window, int active) {
#ifdef __linux__
  return xdo_wait_for_window_active(crossdo, window, active);
#elif _WIN32
  return 0;
#endif
}

int crossdo_enter_text_window(crossdo_t* crossdo, window_t window, const char* string, unsigned int delay) {
#ifdef __linux__
  return xdo_enter_text_window(crossdo, window, string, delay);
#elif _WIN32
  IUnknown* value_pattern_tmp;
  if (FAILED(crossdo->ui_element->GetCurrentPattern(UIA_ValuePatternId, &value_pattern_tmp))) {
    printf("ui_element->GetCurrentPattern(UIA_ValuePatternId, ...) failed");
    return -1;
  }

  IUIAutomationValuePattern* value_pattern = (IUIAutomationValuePattern*)value_pattern_tmp;

  BSTR current_bstr;
  if (FAILED(value_pattern->get_CurrentValue(&current_bstr))) {
    printf("value_pattern->get_CurrentValue(...) failed");
    return -1;
  }
  int current_wslen = SysStringLen(current_bstr);

  int string_wslen = MultiByteToWideChar(CP_UTF8, 0, string, strlen(string), 0, 0);
  BSTR string_bstr = SysAllocStringLen(0, string_wslen);
  MultiByteToWideChar(CP_UTF8, 0, string, strlen(string), string_bstr, string_wslen);

  BSTR new_bstr = SysAllocStringLen(NULL, current_wslen + string_wslen);
  memcpy(new_bstr, current_bstr, current_wslen * sizeof(OLECHAR));
  memcpy(new_bstr + current_wslen, string_bstr, string_wslen * sizeof(OLECHAR));
  new_bstr[current_wslen + string_wslen] = NULL;

  if (FAILED(value_pattern->SetValue(new_bstr))) {
    printf("value_pattern->SetValue(...) failed");
    return -1;
  }

  SysFreeString(new_bstr);
  SysFreeString(string_bstr);
  SysFreeString(current_bstr);

  return 0;
#endif
}

int crossdo_send_keysequence_window(crossdo_t* crossdo, window_t window, const char* keysequence, unsigned int delay) {
#ifdef __linux__
  return xdo_send_keysequence_window(crossdo, window, keysequence, delay);
#elif _WIN32
  return 0;
#endif
}
