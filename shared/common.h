/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Common utilities and helper functions
 */

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include "protocol.h"

namespace ChatUtils {

// ===== Socket Utilities =====

/**
 * Send a message over socket with length prefix
 * Format: [4-byte big-endian length] [JSON payload]
 */
inline bool send_message(int socket, const Message& msg) {
    std::string json = msg.to_json() + "\n";
    uint32_t len = htonl(static_cast<uint32_t>(json.length()));
    
    if (send(socket, &len, sizeof(len), 0) < 0) {
        perror("send (length)");
        return false;
    }
    if (send(socket, json.c_str(), json.length(), 0) < 0) {
        perror("send (payload)");
        return false;
    }
    return true;
}

/**
 * Receive a full message from socket
 * Reads length prefix, then exact number of bytes
 */
inline bool recv_message(int socket, Message& msg) {
    uint32_t len_net = 0;
    int bytes = recv(socket, &len_net, sizeof(len_net), MSG_WAITALL);
    
    if (bytes <= 0) return false;  // Connection closed or error
    
    uint32_t len = ntohl(len_net);
    if (len > 2048) return false;  // Sanity check
    
    std::string buffer(len, '\0');
    bytes = recv(socket, &buffer[0], len, MSG_WAITALL);
    
    if (bytes <= 0) return false;
    
    // Remove trailing newline if present
    if (!buffer.empty() && buffer.back() == '\n') {
        buffer.pop_back();
    }
    
    msg = Message::from_json(buffer);
    return true;
}

// ===== Logging =====

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

inline void log(LogLevel level, const std::string& module, const std::string& msg) {
    const char* level_str[] = {"[DEBUG]", "[INFO]", "[WARN]", "[ERROR]"};
    std::cerr << level_str[static_cast<int>(level)] 
              << " [" << module << "] " << msg << std::endl;
}

#define LOG_INFO(module, msg) log(LogLevel::INFO, module, msg)
#define LOG_WARN(module, msg) log(LogLevel::WARN, module, msg)
#define LOG_ERROR(module, msg) log(LogLevel::ERROR, module, msg)
#define LOG_DEBUG(module, msg) log(LogLevel::DEBUG, module, msg)

}  // namespace ChatUtils

#endif  // COMMON_H
