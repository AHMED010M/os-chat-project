/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Protocol definitions and constants for both socket and shared memory systems
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

// ===== Configuration Constants =====
#define DEFAULT_PORT 5000
#define DEFAULT_SHM_NAME "/os_chat_shm"
#define SHM_MUTEX_NAME "/os_chat_mutex"
#define SHM_COUNT_NAME "/os_chat_count"
#define SHM_BUFFER_SIZE (1024 * 1024)  // 1 MB

// ===== Message Limits =====
#define MAX_USERNAME_LEN 32
#define MAX_TIMESTAMP_LEN 32
#define MAX_MESSAGE_LEN 512
#define MAX_SLOTS 64  // Ring buffer slots

// ===== Socket Protocol =====
// Messages are length-prefixed JSON lines
// Format: [4-byte big-endian length] [JSON payload]
// JSON: {"user":"name","time":"2025-12-08T01:47:00Z","text":"message"}

#define MESSAGE_SEPARATOR '\n'

struct Message {
    char user[MAX_USERNAME_LEN];
    char timestamp[MAX_TIMESTAMP_LEN];
    char text[MAX_MESSAGE_LEN];

    Message() {
        std::memset(user, 0, MAX_USERNAME_LEN);
        std::memset(timestamp, 0, MAX_TIMESTAMP_LEN);
        std::memset(text, 0, MAX_MESSAGE_LEN);
    }

    // Convert to JSON string
    std::string to_json() const {
        std::string json = "{\"user\":\"";
        json += user;
        json += "\",\"time\":\"";
        json += timestamp;
        json += "\",\"text\":\"";
        
        // Escape quotes in text
        for (const char* p = text; *p; ++p) {
            if (*p == '"') json += "\\\"";
            else if (*p == '\\') json += "\\\\";
            else json += *p;
        }
        
        json += "\"}";
        return json;
    }

    // Parse from JSON string
    static Message from_json(const std::string& json) {
        Message msg;
        // Simple JSON parser (production code would use nlohmann/json)
        size_t user_start = json.find("\"user\":\"") + 8;
        size_t user_end = json.find('"', user_start);
        if (user_end != std::string::npos) {
            std::string username = json.substr(user_start, user_end - user_start);
            strncpy(msg.user, username.c_str(), MAX_USERNAME_LEN - 1);
        }
        
        size_t time_start = json.find("\"time\":\"") + 8;
        size_t time_end = json.find('"', time_start);
        if (time_end != std::string::npos) {
            std::string ts = json.substr(time_start, time_end - time_start);
            strncpy(msg.timestamp, ts.c_str(), MAX_TIMESTAMP_LEN - 1);
        }
        
        size_t text_start = json.find("\"text\":\"") + 8;
        size_t text_end = json.rfind('"');
        if (text_end != std::string::npos && text_end > text_start) {
            std::string text = json.substr(text_start, text_end - text_start);
            strncpy(msg.text, text.c_str(), MAX_MESSAGE_LEN - 1);
        }
        
        return msg;
    }

    // Get current timestamp in ISO 8601 format
    static std::string get_current_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }
};

// ===== Shared Memory Layout =====
/*
 * Shared memory segment structure:
 * [
 *   ShmHeader (metadata)
 *   Message[MAX_SLOTS] (ring buffer)
 * ]
 */

struct ShmHeader {
    volatile int read_index;   // Consumer pointer (protected by mutex)
    volatile int write_index;  // Producer pointer (protected by mutex)
    volatile int count;        // Number of unread messages
    int capacity;              // Total slots
    int msg_size;              // Size of each message
};

struct ShmLayout {
    ShmHeader header;
    Message messages[MAX_SLOTS];
};

#endif  // PROTOCOL_H
