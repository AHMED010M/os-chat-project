/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 */

#include "client_handler.h"
#include "../shared/common.h"
#include <unistd.h>
#include <iostream>

using namespace ChatUtils;

// Forward declaration (defined in server.cpp)
extern void broadcast_message(const Message& msg, int exclude_client_id);

ClientHandler::ClientHandler(int socket_fd, int client_id)
    : socket_fd_(socket_fd), client_id_(client_id), 
      connected_(false), should_stop_(false) {}

ClientHandler::~ClientHandler() {
    stop();
    if (socket_fd_ >= 0) {
        close(socket_fd_);
    }
}

void ClientHandler::start() {
    handler_thread_ = std::thread(&ClientHandler::run, this);
}

void ClientHandler::stop() {
    should_stop_ = true;
    if (handler_thread_.joinable()) {
        handler_thread_.join();
    }
}

bool ClientHandler::send_message(const Message& msg) {
    if (!connected_) return false;

    std::lock_guard<std::mutex> lock(send_mutex_);
    return ChatUtils::send_message(socket_fd_, msg);
}

void ClientHandler::run() {
    // First, receive username
    if (!receive_username()) {
        LOG_WARN("ClientHandler", "Failed to receive username from client " + std::to_string(client_id_));
        return;
    }

    connected_ = true;
    LOG_INFO("ClientHandler", "Client " + std::to_string(client_id_) + " connected as \"" + username_ + "\"");

    // Then enter message loop
    message_loop();

    connected_ = false;
    LOG_INFO("ClientHandler", "Client " + std::to_string(client_id_) + " (" + username_ + ") disconnected");
}

bool ClientHandler::receive_username() {
    Message msg;
    if (!ChatUtils::recv_message(socket_fd_, msg)) {
        return false;
    }
    username_ = msg.user;
    return !username_.empty();
}

void ClientHandler::message_loop() {
    Message msg;
    while (!should_stop_ && ChatUtils::recv_message(socket_fd_, msg)) {
        // Update timestamp
        strncpy(msg.timestamp, Message::get_current_timestamp().c_str(), MAX_TIMESTAMP_LEN - 1);
        
        // Broadcast to all clients except sender
        broadcast_message(msg, client_id_);
    }
}
