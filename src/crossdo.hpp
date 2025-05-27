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
#include "windowsx.h"
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
  int nothing;
} crossdo_t;
#endif

crossdo_t* crossdo_new();
void crossdo_free(crossdo_t* crossdo);

int crossdo_get_mouse_location2(
    const crossdo_t* crossdo, int* x_ret, int* y_ret, int* screen_num_ret, window_t* window_ret);
int crossdo_get_caret_location2(
    const crossdo_t* crossdo, int* x_ret, int* y_ret, int* screen_num_ret, window_t* window_ret);


int crossdo_get_pid_window(const crossdo_t* crossdo, window_t window);
int crossdo_get_active_window(const crossdo_t* crossdo, window_t* window_ret);
int crossdo_activate_window(const crossdo_t* crossdo, window_t window);
int crossdo_wait_for_window_active(const crossdo_t* crossdo, window_t window, int active);
int crossdo_enter_text_window(const crossdo_t* crossdo, window_t window, const char* string, unsigned int delay);
int crossdo_send_keysequence_window(const crossdo_t* crossdo, window_t window, const char* keysequence, unsigned int delay);

#ifdef __cplusplus
}
#endif
