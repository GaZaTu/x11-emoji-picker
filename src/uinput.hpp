#pragma once

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unicode/schriter.h>
#include <unicode/unistr.h>
#include <unistd.h>
#include <vector>

namespace uinput {
int open(const char* name) {
  int fd = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (fd < 1) {
    return fd;
  }

  uinput_user_dev uinput;
  memset(&uinput, 0, sizeof(uinput));
  snprintf(uinput.name, UINPUT_MAX_NAME_SIZE, name);
  uinput.id.bustype = BUS_VIRTUAL;
  uinput.id.vendor = 0x1;
  uinput.id.product = 0x1;
  uinput.id.version = 1;

  ioctl(fd, UI_SET_EVBIT, EV_SYN);
  ioctl(fd, UI_SET_EVBIT, EV_KEY);

  for (int i = 0; i < 256; i++) {
    ioctl(fd, UI_SET_KEYBIT, i);
  }

  write(fd, &uinput, sizeof(uinput));
  ioctl(fd, UI_DEV_CREATE);

  return fd;
}

void close(int fd) {
  ioctl(fd, UI_DEV_DESTROY);
  ::close(fd);
}

constexpr int32_t KEY_PRESS = 1;
constexpr int32_t KEY_RELEASE = 0;
input_event newInputEvent(uint16_t type, uint16_t code, int32_t value) {
  struct input_event event;
  memset(&event, 0, sizeof(event));
  gettimeofday(&event.time, NULL);
  event.type = type;
  event.code = code;
  event.value = value;

  return event;
}

void writeInputEvent(int fd, uint16_t type, uint16_t code, int32_t value, int timeout = 20000) {
  input_event ev = newInputEvent(type, code, value);

  write(fd, &ev, sizeof(ev));
  usleep(timeout);
}

void writeKeypress(int fd, int key, int mod = -1, int syn = -1) {
  if (mod == KEY_LEFTCTRL || mod == (KEY_LEFTCTRL & KEY_LEFTSHIFT)) {
    writeInputEvent(fd, EV_KEY, KEY_LEFTCTRL, KEY_PRESS);
  }

  if (mod == KEY_LEFTSHIFT || mod == (KEY_LEFTCTRL & KEY_LEFTSHIFT)) {
    writeInputEvent(fd, EV_KEY, KEY_LEFTSHIFT, KEY_PRESS);
  }

  writeInputEvent(fd, EV_KEY, key, KEY_PRESS);
  writeInputEvent(fd, EV_KEY, key, KEY_RELEASE);

  if (mod == KEY_LEFTSHIFT || mod == (KEY_LEFTCTRL & KEY_LEFTSHIFT)) {
    writeInputEvent(fd, EV_KEY, KEY_LEFTSHIFT, KEY_RELEASE);
  }

  if (mod == KEY_LEFTCTRL || mod == (KEY_LEFTCTRL & KEY_LEFTSHIFT)) {
    writeInputEvent(fd, EV_KEY, KEY_LEFTCTRL, KEY_RELEASE);
  }

  if (syn == SYN_REPORT) {
    writeInputEvent(fd, EV_SYN, SYN_REPORT, 0);
  }
}
} // namespace uinput
