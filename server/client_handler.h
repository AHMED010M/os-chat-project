/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Client connection handler for the socket chat server
 */

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include "../shared/protocol.h"

// Forward declaration for server broadcast
void broadcast_message(const Message& msg, int exclude_client_id = -1);

class ClientHandler {
public:
    ClientHandler(int socket_fd, int client_id);
    ~ClientHandler();

    // Start the handler thread
    void start();

    // Stop the handler (graceful shutdown)
    void stop();

    // Get client information
    int get_id() const { return client_id_; }
    const std::string& get_username() const { return username_; }
    int get_socket() const { return socket_fd_; }
    bool is_connected() const { return connected_; }

    // Send a message to this client
    bool send_message(const Message& msg);

private:
    // Thread function
    void run();

    // Read username from client
    bool receive_username();

    // Message loop
    void message_loop();

    int socket_fd_;
    int client_id_;
    std::string username_;
    std::atomic<bool> connected_;
    std::atomic<bool> should_stop_;
    std::thread handler_thread_;
    std::mutex send_mutex_;  // Protect socket writes
};

#endif  // CLIENT_HANDLER_H
