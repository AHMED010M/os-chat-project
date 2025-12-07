/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Shared memory client for GUI (System B - Local Chat)
 */

#ifndef SHM_CLIENT_H
#define SHM_CLIENT_H

#include <QObject>
#include <QString>
#include <thread>
#include <atomic>
#include <semaphore.h>
#include "../shared/protocol.h"

class ShmClient : public QObject {
    Q_OBJECT

public:
    ShmClient(QObject* parent = nullptr);
    ~ShmClient();

    // Join shared memory chat room
    bool join_room(const QString& shm_name, const QString& username);

    // Leave room
    void leave_room();

    // Check if joined
    bool is_joined() const { return joined_; }

    // Send a message
    bool send_message(const QString& text);

private:
    void read_loop();
    bool initialize_shared_memory(const QString& shm_name);
    bool write_to_buffer(const Message& msg);
    bool read_from_buffer(Message& msg);

    int shm_fd_;
    void* shm_ptr_;
    ShmLayout* layout_;
    sem_t* mutex_sem_;
    sem_t* count_sem_;
    
    std::atomic<bool> joined_;
    std::atomic<bool> should_stop_;
    std::thread read_thread_;
    QString username_;
    int last_read_index_;

signals:
    void joined();
    void left();
    void message_received(QString user, QString timestamp, QString text);
    void error_occurred(QString error_msg);
};

#endif  // SHM_CLIENT_H
