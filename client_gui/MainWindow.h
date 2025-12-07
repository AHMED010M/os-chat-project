/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Main GUI window for chat client (supports both Socket and Shared Memory modes)
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <memory>
#include "SocketClient.h"
#include "ShmClient.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_mode_changed(int index);
    void on_connect_button_clicked();
    void on_send_button_clicked();
    void on_message_received(QString user, QString timestamp, QString text);
    void on_connected();
    void on_disconnected();
    void on_error(QString error);

private:
    void setup_ui();
    void update_ui_for_mode();
    void append_message(const QString& user, const QString& timestamp, const QString& text);

    // UI components
    QComboBox* mode_combo_;
    QLineEdit* ip_input_;
    QSpinBox* port_input_;
    QLineEdit* shm_name_input_;
    QLineEdit* username_input_;
    QPushButton* connect_button_;
    
    QTextEdit* messages_display_;
    QLineEdit* message_input_;
    QPushButton* send_button_;
    
    QLabel* status_label_;
    QLabel* mode_label_;

    // Clients
    std::unique_ptr<SocketClient> socket_client_;
    std::unique_ptr<ShmClient> shm_client_;

    // State
    bool is_connected_;
    int current_mode_;  // 0 = Socket, 1 = Shared Memory
};

#endif  // MAINWINDOW_H
