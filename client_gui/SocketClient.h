/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Socket client for GUI (System A - Network Chat)
 */

#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <QObject>
#include <QString>
#include <QThread>
#include <atomic>
#include <thread>
#include "../shared/protocol.h"

class SocketClient : public QObject {
    Q_OBJECT

public:
    SocketClient(QObject* parent = nullptr);
    ~SocketClient();

    // Connect to server
    bool connect_to_server(const QString& host, int port, const QString& username);

    // Disconnect from server
    void disconnect();

    // Check if connected
    bool is_connected() const { return connected_; }

    // Send a message
    bool send_message(const QString& text);

private:
    void receive_loop();

    int socket_fd_;
    std::atomic<bool> connected_;
    std::atomic<bool> should_stop_;
    std::thread receive_thread_;
    QString username_;

signals:
    void connected();
    void disconnected();
    void message_received(QString user, QString timestamp, QString text);
    void error_occurred(QString error_msg);
};

#endif  // SOCKET_CLIENT_H
