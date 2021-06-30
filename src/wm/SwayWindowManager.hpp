#pragma once

#include "../WindowManager.hpp"
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <dlfcn.h>
#include <memory>

// echo $(swaymsg -t get_tree) | jq ".. | select(.type?) | select(.focused==true).id"
// swaymsg "[app_id=firefox] focus"

// SWAYSOCK

constexpr const char MAGIC_STRING[] = "i3-ipc";

constexpr int RUN_COMMAND = 0;
constexpr int GET_TREE = 4;

class SwayWindowManager : public wm::WindowManager {
public:
  SwayWindowManager() {
    const char* swaysock = getenv("SWAYSOCK");
    if (swaysock == nullptr) {
      return;
    }

    _socket.connectToServer(swaysock);
  }

  std::string name() override {
    return "SwayWindowManager";
  }

  operator bool() override {
    return !_socket.serverName().isNull();
  }

  wm::WId activeWindow() override {
    QString payload = "";
    QByteArray input = createMessage(GET_TREE, "");

    _socket.write(input);
    _socket.waitForReadyRead();

    QByteArray output = _socket.readAll();
    QJsonDocument document = parseReply(output);
    // QJsonObject root = document.object();

    qDebug() << document;

    return 0;
  }

  void activate(wm::WId window) override {
    QString payload = QString{"[window_id=%1] focus"}.arg(window);
    QByteArray input = createMessage(RUN_COMMAND, payload);

    _socket.write(input);
  }

private:
  QLocalSocket _socket;

  static QByteArray createMessage(int type, const QString& payload) {
    QByteArray input;
    QDataStream istream{&input, QIODevice::WriteOnly};
    istream << MAGIC_STRING;
    istream << (int32_t)payload.length();
    istream << (int32_t)type;
    istream << payload;

    return input;
  }

  static QJsonDocument parseReply(QByteArray& output) {
    // QDataStream ostream{&output, QIODevice::ReadOnly};
    // ostream.skipRawData(sizeof(MAGIC_STRING));
    // ostream.skipRawData(sizeof(int32_t));
    // ostream.skipRawData(sizeof(int32_t));

    output.remove(0, sizeof(MAGIC_STRING));
    output.remove(0, sizeof(int32_t));
    output.remove(0, sizeof(int32_t));

    return QJsonDocument::fromJson(output);
  }
};