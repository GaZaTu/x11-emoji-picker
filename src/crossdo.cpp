#include "crossdo.hpp"

#ifdef __cplusplus
extern "C" {
#endif

crossdo_t* crossdo_new() {
#if !_WIN32
  return xdo_new(NULL);
#else
  return (crossdo_t*)malloc(sizeof(crossdo_t));
#endif
}

void crossdo_free(crossdo_t* crossdo) {
#if !_WIN32
  xdo_free(crossdo);
#else
  free(crossdo);
#endif
}

int crossdo_get_mouse_location2(
    const crossdo_t* crossdo, int* x_ret, int* y_ret, int* screen_num_ret, window_t* window_ret) {
#if !_WIN32
  return xdo_get_mouse_location2(crossdo, x_ret, y_ret, screen_num_ret, window_ret);
#else
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
    const crossdo_t* crossdo, int* x_ret, int* y_ret, int* screen_num_ret, window_t* window_ret) {
#if !_WIN32
  *x_ret = 0;
  *y_ret = 0;
  *screen_num_ret = 0;
  *window_ret = 0;

  return 0;
#else
  *x_ret = 0;
  *y_ret = 0;
  *screen_num_ret = 0;
  *window_ret = 0;

  return 0;
#endif
}

int crossdo_get_pid_window(const crossdo_t* crossdo, window_t window) {
#if !_WIN32
  return xdo_get_pid_window(crossdo, window);
#else
  unsigned long pid = 0;
  GetWindowThreadProcessId(window, &pid);

  return pid;
#endif
}

int crossdo_get_active_window(const crossdo_t* crossdo, window_t* window_ret) {
#if !_WIN32
  return xdo_get_active_window(crossdo, window_ret);
#else
  *window_ret = GetForegroundWindow();

  return 0;
#endif
}

int crossdo_activate_window(const crossdo_t* crossdo, window_t window) {
#if !_WIN32
  return xdo_activate_window(crossdo, window);
#else
  window_t current_window = GetForegroundWindow();

  unsigned long process_thread = GetProcessId();
  unsigned long current_thread = GetWindowThreadProcessId(current_window, NULL);
  unsigned long window_thread = GetWindowThreadProcessId(window, NULL);

  if (process_thread == current_thread && process_thread != window_thread) {
    AttachThreadInput(window_thread, process_thread, TRUE);
  }

  SetActiveWindow(window);

  if (process_thread == window_thread && process_thread != current_thread) {
    AttachThreadInput(current_thread, process_thread, FALSE);
  }

  return 0;
#endif
}

int crossdo_wait_for_window_active(const crossdo_t* crossdo, window_t window, int active) {
#if !_WIN32
  return xdo_wait_for_window_active(crossdo, window, active);
#else
  Sleep(25);

  return 0;
#endif
}

int crossdo_enter_text_window(const crossdo_t* crossdo, window_t window, const char* string, unsigned int delay) {
#if !_WIN32
  return xdo_enter_text_window(crossdo, window, string, delay);
#else
  // string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  size_t input_len = strlen(string) * 2;
  INPUT* input = (INPUT*)malloc(sizeof(INPUT) * input_len);

  window_t current_window = GetForegroundWindow();
  unsigned long current_thread = GetWindowThreadProcessId(current_window, NULL);
  unsigned long window_thread = GetWindowThreadProcessId(window, NULL);

  AttachThreadInput(window_thread, current_thread, TRUE);

  // SetActiveWindow(window);

  for (int i = 0; i < (input_len / 2); i++) {
    INPUT* down = &input[(i * 2) + 0];
    INPUT* up = &input[(i * 2) + 1];

    down->type = INPUT_KEYBOARD;
    down->ki.wScan = string[i];
    down->ki.time = 0;
    down->ki.dwExtraInfo = NULL;
    down->ki.wVk = 0;
    down->ki.dwFlags = KEYEVENTF_UNICODE | 0;

    up->type = INPUT_KEYBOARD;
    up->ki.wScan = string[i];
    up->ki.time = 0;
    up->ki.dwExtraInfo = NULL;
    up->ki.wVk = 0;
    up->ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    // INPUT input[2];

    // input[0].type = INPUT_KEYBOARD;
    // input[0].ki.wScan = string[i];
    // input[0].ki.time = 0;
    // input[0].ki.dwExtraInfo = NULL;
    // input[0].ki.wVk = 0;
    // input[0].ki.dwFlags = KEYEVENTF_UNICODE | 0;

    // input[1].type = INPUT_KEYBOARD;
    // input[1].ki.wScan = string[i];
    // input[1].ki.time = 0;
    // input[1].ki.dwExtraInfo = NULL;
    // input[1].ki.wVk = 0;
    // input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    // SendInput(2, input, sizeof(INPUT));
  }

  SendInput(input_len, input, sizeof(INPUT));

  SetActiveWindow(current_window);

  AttachThreadInput(window_thread, current_thread, FALSE);

  free(input);

  return 0;
#endif
}

int crossdo_send_keysequence_window(const crossdo_t* crossdo, window_t window, const char* keysequence, unsigned int delay) {
#if !_WIN32
  return xdo_send_keysequence_window(crossdo, window, keysequence, delay);
#else
  return 0;
#endif
}

#ifdef __cplusplus
}
#endif
