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

// short hexCharToKeycode(char chr) {
//   switch (chr) {
//   case '0':
//     return KEY_0;
//   case '1':
//     return KEY_1;
//   case '2':
//     return KEY_2;
//   case '3':
//     return KEY_3;
//   case '4':
//     return KEY_4;
//   case '5':
//     return KEY_5;
//   case '6':
//     return KEY_6;
//   case '7':
//     return KEY_7;
//   case '8':
//     return KEY_8;
//   case '9':
//     return KEY_9;
//   case 'a':
//     return KEY_A;
//   case 'b':
//     return KEY_B;
//   case 'c':
//     return KEY_C;
//   case 'd':
//     return KEY_D;
//   case 'e':
//     return KEY_E;
//   case 'f':
//     return KEY_F;
//   }
// }

// std::vector<input_event> unicodeToInputEvents(const std::string& str) {
//   std::vector<input_event> result;
//   std::stringstream stream;

//   icu::UnicodeString ustr{str.data(), str.length(), "utf-8"};
//   icu::StringCharacterIterator iterator{ustr};
//   while (iterator.hasNext()) {
//     result.push_back(newInputEvent(EV_KEY, KEY_LEFTCTRL, KEY_PRESS));
//     result.push_back(newInputEvent(EV_KEY, KEY_LEFTSHIFT, KEY_PRESS));
//     result.push_back(newInputEvent(EV_KEY, KEY_U, KEY_PRESS));
//     result.push_back(newInputEvent(EV_KEY, KEY_U, KEY_RELEASE));
//     result.push_back(newInputEvent(EV_KEY, KEY_LEFTSHIFT, KEY_RELEASE));
//     result.push_back(newInputEvent(EV_KEY, KEY_LEFTCTRL, KEY_RELEASE));
//     result.push_back(newInputEvent(EV_SYN, SYN_REPORT, 0));

//     stream.str({});
//     stream << std::hex << iterator.next32PostInc();

//     for (char chr : stream.str()) {
//       short key = hexCharToKeycode(chr);

//       result.push_back(newInputEvent(EV_KEY, key, KEY_PRESS));
//       result.push_back(newInputEvent(EV_KEY, key, KEY_RELEASE));
//       // result.push_back(newInputEvent(EV_SYN, SYN_REPORT, 0));
//     }

//     result.push_back(newInputEvent(EV_KEY, KEY_ENTER, KEY_PRESS));
//     result.push_back(newInputEvent(EV_KEY, KEY_ENTER, KEY_RELEASE));
//     result.push_back(newInputEvent(EV_SYN, SYN_REPORT, 0));
//   }

//   return result;
// }

struct file {
public:
  int fd;

  file(int f) : fd(f) {
  }

  file(const char* n) {
    fd = uinput::open(n);
  }

  ~file() {
    uinput::close(fd);
  }

  void writeInputEvent(uint16_t type, uint16_t code, int32_t value, int timeout = 20000) {
    uinput::writeInputEvent(fd, type, code, value, timeout);
  }

  void writeKeypress(int key, int mod = -1, int syn = -1) {
    uinput::writeKeypress(fd, key, mod, syn);
  }

  operator bool() {
    return fd > 0;
  }
};
} // namespace uinput
