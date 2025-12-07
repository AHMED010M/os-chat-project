/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 */

#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QDebug>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), is_connected_(false), current_mode_(0) {
    setWindowTitle("OS Chat Client - Socket & Shared Memory");
    setGeometry(100, 100, 800, 600);

    socket_client_ = std::make_unique<SocketClient>();
    shm_client_ = std::make_unique<ShmClient>();

    // Connect signals
    connect(socket_client_.get(), &SocketClient::connected, this, &MainWindow::on_connected);
    connect(socket_client_.get(), &SocketClient::disconnected, this, &MainWindow::on_disconnected);
    connect(socket_client_.get(), &SocketClient::message_received, this, &MainWindow::on_message_received);
    connect(socket_client_.get(), QOverload<QString>::of(&SocketClient::error_occurred), this, &MainWindow::on_error);

    connect(shm_client_.get(), &ShmClient::joined, this, &MainWindow::on_connected);
    connect(shm_client_.get(), &ShmClient::left, this, &MainWindow::on_disconnected);
    connect(shm_client_.get(), &ShmClient::message_received, this, &MainWindow::on_message_received);
    connect(shm_client_.get(), QOverload<QString>::of(&ShmClient::error_occurred), this, &MainWindow::on_error);

    setup_ui();
}

MainWindow::~MainWindow() {}

void MainWindow::setup_ui() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* main_layout = new QVBoxLayout(central);

    // ===== Connection Settings Group =====
    QGroupBox* settings_group = new QGroupBox("Connection Settings", this);
    QVBoxLayout* settings_layout = new QVBoxLayout(settings_group);

    // Mode selection
    QHBoxLayout* mode_layout = new QHBoxLayout();
    mode_label_ = new QLabel("Mode:");
    mode_combo_ = new QComboBox();
    mode_combo_->addItem("Socket (Network)");
    mode_combo_->addItem("Shared Memory (Local)");
    mode_layout->addWidget(mode_label_);
    mode_layout->addWidget(mode_combo_);
    mode_layout->addStretch();
    settings_layout->addLayout(mode_layout);
    connect(mode_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::on_mode_changed);

    // Server IP/Port (Socket mode)
    QHBoxLayout* ip_layout = new QHBoxLayout();
    ip_layout->addWidget(new QLabel("Server IP:"));
    ip_input_ = new QLineEdit("127.0.0.1");
    ip_layout->addWidget(ip_input_);
    ip_layout->addWidget(new QLabel("Port:"));
    port_input_ = new QSpinBox();
    port_input_->setRange(1024, 65535);
    port_input_->setValue(5000);
    ip_layout->addWidget(port_input_);
    settings_layout->addLayout(ip_layout);

    // Shared Memory Name (SHM mode)
    QHBoxLayout* shm_layout = new QHBoxLayout();
    shm_layout->addWidget(new QLabel("SHM Name:"));
    shm_name_input_ = new QLineEdit("/os_chat_shm");
    shm_layout->addWidget(shm_name_input_);
    shm_layout->addStretch();
    settings_layout->addLayout(shm_layout);

    // Username
    QHBoxLayout* user_layout = new QHBoxLayout();
    user_layout->addWidget(new QLabel("Username:"));
    username_input_ = new QLineEdit();
    username_input_->setPlaceholderText("Enter your username");
    user_layout->addWidget(username_input_);
    user_layout->addStretch();
    settings_layout->addLayout(user_layout);

    // Connect button
    QHBoxLayout* btn_layout = new QHBoxLayout();
    connect_button_ = new QPushButton("Connect");
    btn_layout->addStretch();
    btn_layout->addWidget(connect_button_);
    settings_layout->addLayout(btn_layout);
    connect(connect_button_, &QPushButton::clicked, this, &MainWindow::on_connect_button_clicked);

    main_layout->addWidget(settings_group);

    // ===== Chat Area =====
    messages_display_ = new QTextEdit();
    messages_display_->setReadOnly(true);
    messages_display_->setStyleSheet("QTextEdit { background-color: #f5f5f5; }");
    main_layout->addWidget(new QLabel("Messages:"));
    main_layout->addWidget(messages_display_);

    // ===== Input Area =====
    QHBoxLayout* input_layout = new QHBoxLayout();
    message_input_ = new QLineEdit();
    message_input_->setEnabled(false);
    message_input_->setPlaceholderText("Type message and press Send...");
    input_layout->addWidget(message_input_);
    
    send_button_ = new QPushButton("Send");
    send_button_->setEnabled(false);
    send_button_->setMaximumWidth(80);
    input_layout->addWidget(send_button_);
    connect(send_button_, &QPushButton::clicked, this, &MainWindow::on_send_button_clicked);
    connect(message_input_, &QLineEdit::returnPressed, this, &MainWindow::on_send_button_clicked);

    main_layout->addLayout(input_layout);

    // ===== Status Bar =====
    status_label_ = new QLabel("Disconnected");
    status_label_->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    main_layout->addWidget(status_label_);

    // Apply initial mode
    update_ui_for_mode();
}

void MainWindow::update_ui_for_mode() {
    current_mode_ = mode_combo_->currentIndex();
    bool is_socket_mode = (current_mode_ == 0);

    ip_input_->setEnabled(is_socket_mode && !is_connected_);
    port_input_->setEnabled(is_socket_mode && !is_connected_);
    shm_name_input_->setEnabled(!is_socket_mode && !is_connected_);
}

void MainWindow::on_mode_changed(int index) {
    // Disconnect if already connected
    if (is_connected_) {
        if (current_mode_ == 0) {
            socket_client_->disconnect();
        } else {
            shm_client_->leave_room();
        }
    }
    update_ui_for_mode();
}

void MainWindow::on_connect_button_clicked() {
    if (is_connected_) {
        if (current_mode_ == 0) {
            socket_client_->disconnect();
        } else {
            shm_client_->leave_room();
        }
        return;
    }

    QString username = username_input_->text().trimmed();
    if (username.isEmpty()) {
        on_error("Please enter a username");
        return;
    }

    bool success = false;
    if (current_mode_ == 0) {
        // Socket mode
        QString host = ip_input_->text().trimmed();
        int port = port_input_->value();
        success = socket_client_->connect_to_server(host, port, username);
    } else {
        // Shared Memory mode
        QString shm_name = shm_name_input_->text().trimmed();
        success = shm_client_->join_room(shm_name, username);
    }

    if (!success) {
        on_error("Connection failed");
    }
}

void MainWindow::on_send_button_clicked() {
    QString text = message_input_->text().trimmed();
    if (text.isEmpty()) return;

    bool success = false;
    if (current_mode_ == 0) {
        success = socket_client_->send_message(text);
    } else {
        success = shm_client_->send_message(text);
    }

    if (success) {
        // Show our message immediately
        append_message(username_input_->text(), QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ssZ"), text);
        message_input_->clear();
    } else {
        on_error("Failed to send message");
    }
}

void MainWindow::on_message_received(QString user, QString timestamp, QString text) {
    append_message(user, timestamp, text);
}

void MainWindow::on_connected() {
    is_connected_ = true;
    status_label_->setText("Connected");
    status_label_->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    connect_button_->setText("Disconnect");
    message_input_->setEnabled(true);
    send_button_->setEnabled(true);
    mode_combo_->setEnabled(false);
    username_input_->setEnabled(false);
    ip_input_->setEnabled(false);
    port_input_->setEnabled(false);
    shm_name_input_->setEnabled(false);
    messages_display_->clear();
    append_message("[System]", "", "Connected successfully");
}

void MainWindow::on_disconnected() {
    is_connected_ = false;
    status_label_->setText("Disconnected");
    status_label_->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    connect_button_->setText("Connect");
    message_input_->setEnabled(false);
    send_button_->setEnabled(false);
    mode_combo_->setEnabled(true);
    username_input_->setEnabled(true);
    update_ui_for_mode();
    append_message("[System]", "", "Disconnected");
}

void MainWindow::on_error(QString error) {
    append_message("[Error]", "", error);
}

void MainWindow::append_message(const QString& user, const QString& timestamp, const QString& text) {
    QString formatted;
    if (!timestamp.isEmpty()) {
        formatted = QString("[%1] %2: %3").arg(timestamp, user, text);
    } else {
        formatted = QString("%1: %2").arg(user, text);
    }
    messages_display_->append(formatted);
}
