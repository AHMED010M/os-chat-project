/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Socket System Tests
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../shared/protocol.h"
#include "../shared/common.h"

using namespace ChatUtils;

int simple_server(int port) {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    assert(server >= 0);

    int reuse = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    assert(bind(server, (struct sockaddr*)&addr, sizeof(addr)) == 0);
    assert(listen(server, 1) == 0);

    int client = accept(server, nullptr, nullptr);
    assert(client >= 0);

    Message msg;
    assert(recv_message(client, msg));
    std::cout << "[test_socket] Received: " << msg.user << " -> " << msg.text << std::endl;
    assert(strcmp(msg.user, "test_user") == 0);
    assert(strcmp(msg.text, "Hello from test") == 0);

    close(client);
    close(server);
    return 0;
}

void test_message_protocol() {
    std::cout << "\n=== Test: Message Protocol ===" << std::endl;

    Message msg;
    strncpy(msg.user, "alice", MAX_USERNAME_LEN - 1);
    strncpy(msg.timestamp, "2025-12-08T01:47:00Z", MAX_TIMESTAMP_LEN - 1);
    strncpy(msg.text, "Hello, world!", MAX_MESSAGE_LEN - 1);

    std::string json = msg.to_json();
    std::cout << "JSON: " << json << std::endl;
    assert(!json.empty());
    assert(json.find("\"user\":\"alice\"") != std::string::npos);

    Message msg2 = Message::from_json(json);
    assert(strcmp(msg2.user, "alice") == 0);
    assert(strcmp(msg2.text, "Hello, world!") == 0);

    std::cout << "✓ Message protocol test passed" << std::endl;
}

void test_timestamp() {
    std::cout << "\n=== Test: Timestamp Generation ===" << std::endl;

    std::string ts = Message::get_current_timestamp();
    std::cout << "Timestamp: " << ts << std::endl;
    assert(ts.length() > 0);
    assert(ts.find("T") != std::string::npos);
    assert(ts.find("Z") != std::string::npos);

    std::cout << "✓ Timestamp test passed" << std::endl;
}

void test_socket_communication() {
    std::cout << "\n=== Test: Socket Communication ===" << std::endl;

    // Start server in a thread
    std::thread server_thread(simple_server, 15000);

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Client connects
    int client = socket(AF_INET, SOCK_STREAM, 0);
    assert(client >= 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(15000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    int ret = connect(client, (struct sockaddr*)&addr, sizeof(addr));
    assert(ret == 0);

    // Send message
    Message msg;
    strncpy(msg.user, "test_user", MAX_USERNAME_LEN - 1);
    strncpy(msg.timestamp, Message::get_current_timestamp().c_str(), MAX_TIMESTAMP_LEN - 1);
    strncpy(msg.text, "Hello from test", MAX_MESSAGE_LEN - 1);

    assert(send_message(client, msg));

    close(client);
    server_thread.join();

    std::cout << "✓ Socket communication test passed" << std::endl;
}

int main() {
    std::cout << "\n========== Socket System Tests ==========\n" << std::endl;

    try {
        test_message_protocol();
        test_timestamp();
        test_socket_communication();

        std::cout << "\n========== All Tests Passed! ==========\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed: " << e.what() << std::endl;
        return 1;
    }
}
