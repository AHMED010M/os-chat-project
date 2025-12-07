/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Socket Chat Server - System A
 * Multi-threaded TCP server that broadcasts messages to all connected clients
 */

#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client_handler.h"
#include "../shared/protocol.h"
#include "../shared/common.h"

using namespace ChatUtils;

// Global state (protected by mutex)
static std::vector<std::shared_ptr<ClientHandler>> clients;
static std::mutex clients_mutex;
static int server_socket = -1;
static std::atomic<bool> running(true);

void signal_handler(int sig) {
    if (sig == SIGINT) {
        LOG_INFO("Server", "Received SIGINT, shutting down...");
        running = false;
    }
}

void broadcast_message(const Message& msg, int exclude_client_id = -1) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    for (auto& client : clients) {
        if (client && client->is_connected() && 
            (exclude_client_id < 0 || client->get_id() != exclude_client_id)) {
            client->send_message(msg);
        }
    }
}

void accept_loop() {
    int client_id = 0;
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (running) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client_socket < 0) {
            if (running) {
                perror("accept");
            }
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        LOG_INFO("Server", "New connection from " + std::string(client_ip) + ":" + 
                           std::to_string(ntohs(client_addr.sin_port)));

        auto handler = std::make_shared<ClientHandler>(client_socket, client_id++);
        handler->start();

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(handler);
        }
    }
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        }
    }

    // Setup signal handler
    std::signal(SIGINT, signal_handler);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return 1;
    }

    // Allow reusing the address
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(server_socket);
        return 1;
    }

    // Bind to port
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        return 1;
    }

    // Start listening
    if (listen(server_socket, 5) < 0) {
        perror("listen");
        close(server_socket);
        return 1;
    }

    LOG_INFO("Server", "Chat server started on 0.0.0.0:" + std::to_string(port));
    LOG_INFO("Server", "Waiting for connections... (Press Ctrl+C to stop)");

    // Accept client connections
    accept_loop();

    // Cleanup
    LOG_INFO("Server", "Shutting down server...");
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto& client : clients) {
            if (client) {
                client->stop();
            }
        }
        clients.clear();
    }

    close(server_socket);
    LOG_INFO("Server", "Server stopped");

    return 0;
}
