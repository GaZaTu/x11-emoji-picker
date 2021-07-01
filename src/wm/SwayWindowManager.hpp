#pragma once

#include "../WindowManager.hpp"
#include <QDataStream>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
// #include <QLocalSocket>
#include <QCoreApplication>
#include <QProcess>
#include <memory>

constexpr const char SWAYSOCK[] = "SWAYSOCK";
constexpr const char MAGIC_STRING[] = "i3-ipc";

constexpr int RUN_COMMAND = 0;
constexpr int GET_TREE = 4;

QJsonObject getFocusedNodeInTree(QJsonObject& node) {
  if (node["focused"].toBool() == true) {
    return node;
  }

  for (QJsonValue child : node["nodes"].toArray()) {
    QJsonObject childObject = child.toObject();
    QJsonObject focusedChild = getFocusedNodeInTree(childObject);

    if (!focusedChild.isEmpty()) {
      return focusedChild;
    }
  }

  return {};
}

class SwayWindowManager : public wm::WindowManager {
public:
  SwayWindowManager() {
    const char* swaysock = getenv(SWAYSOCK);
    if (swaysock == nullptr) {
      return;
    }

    // _socket.connectToServer(swaysock);
    // _socket.waitForConnected();

    _valid = true;
  }

  std::string name() override {
    return "SwayWindowManager";
  }

  operator bool() override {
    return _valid;
  }

  wm::WId activeWindow() override {
    // QString payload = "";
    // QByteArray input = createMessage(GET_TREE, payload);

    // _socket.write(input);
    // _socket.waitForBytesWritten();
    // _socket.waitForReadyRead();

    QProcess process;
    process.setProgram("swaymsg");
    process.setArguments({"-t", "get_tree"});
    process.start();
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QJsonDocument document = QJsonDocument::fromJson(output);

    // QByteArray output = _socket.readAll();
    // QJsonDocument document = parseReply(output);
    QJsonObject root = document.object();

    QJsonObject activeWindow = getFocusedNodeInTree(root);
    if (activeWindow.isEmpty()) {
      return 0;
    }

    return activeWindow["id"].toInt();
  }

  void activate(wm::WId window) override {
    QString payload = QString{"[con_id=%1] focus"}.arg(window);
    if (window == -1) {
      payload = QString{"[pid=%1] focus"}.arg(QCoreApplication::applicationPid());
    }

    // QByteArray input = createMessage(RUN_COMMAND, payload);

    // _socket.write(input);
    // _socket.waitForBytesWritten();
    // _socket.waitForReadyRead();

    // _socket.readAll();

    QProcess process;
    process.setProgram("swaymsg");
    process.setArguments({payload});
    process.start();
    process.waitForFinished();
  }

  wm::WId getWIdForQWindow(const QWidget& window) override {
    return -1;
  }

private:
  // QLocalSocket _socket;
  bool _valid = false;

  // static QByteArray createMessage(int type, const QString& payload) {
  //   QByteArray input;
  //   QDataStream istream{&input, QIODevice::WriteOnly};
  //   istream << MAGIC_STRING;
  //   istream << (int32_t)payload.length();
  //   istream << (int32_t)type;
  //   istream << payload;

  //   return input;
  // }

  // static QJsonDocument parseReply(QByteArray& output) {
  //   output.remove(0, sizeof(MAGIC_STRING));
  //   output.remove(0, sizeof(int32_t));
  //   output.remove(0, sizeof(int32_t));

  //   return QJsonDocument::fromJson(output);
  // }
};
