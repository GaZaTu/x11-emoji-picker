#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
#include "xdo.h"
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
#define NONLS 1
#define NOMB 1
#define NOMEMMGR 1
#define NOMETAFILE 1
#define NOMINMAX 1
#define NOMSG 1
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
#endif

#ifdef __linux__
typedef xdo_t crossdo_t;
#elif _WIN32
    typedef struct {
  int x;
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
  *result = GetForegroundWindow();

  return 0;
#endif
}

int crossdo_enter_text_window(crossdo_t* crossdo, window_t window, const char* string, unsigned int delay) {
#ifdef __linux__
  return xdo_enter_text_window(crossdo, window, string, delay);
#elif _WIN32
  // SendMessageW(window, WM_SETTEXT, NULL, (LPARAM)string);

  window_t this_window = GetForegroundWindow();

  OpenClipboard(this_window);

  unsigned int clipboard_formats_count = CountClipboardFormats();
  unsigned int* clipboard_formats = (unsigned int*)malloc(sizeof(unsigned int) * clipboard_formats_count);
  unsigned int clipboard_format = 0;
  for (int i = 0; (clipboard_format = EnumClipboardFormats(clipboard_format)) != 0; i++) {
    clipboard_formats[i] = clipboard_format;
  }

  char** clipboard_data = (char**)malloc(sizeof(char*) * clipboard_formats_count);
  for (int i = 0; i < clipboard_formats_count; i++) {
    char* format_data = (char*)GetClipboardData(clipboard_formats[i]);
    size_t format_data_len = strlen(format_data) + 1;

    clipboard_data[i] = (char*)malloc(sizeof(unsigned char) * format_data_len);

    strcpy_s(clipboard_data[i], format_data_len, format_data);
  }

  size_t clipboard_text_len = sizeof(char) * (strlen(string) + 1);
  void* clipboard_text_ptr = GlobalAlloc(NULL, clipboard_text_len);
  char* clipboard_text = (char*)GlobalLock(clipboard_text_ptr);
  wcscpy((wchar_t*)clipboard_text, (wchar_t*)string);

  EmptyClipboard();
  SetClipboardData(CF_UNICODETEXT, clipboard_text_ptr);

  GlobalUnlock(clipboard_text_ptr);

  CloseClipboard();
  SendMessageW(window, WM_PASTE, NULL, NULL);
  OpenClipboard(this_window);

  for (int i = 0; i < clipboard_formats_count; i++) {
    SetClipboardData(clipboard_formats[i], clipboard_data[i]);
  }

  free(clipboard_formats);
  free(clipboard_data);

  CloseClipboard();

  return 0;
#endif
}

#ifdef __cplusplus
}
#endif
