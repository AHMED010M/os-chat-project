/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 */

#include "ShmClient.h"
#include "../shared/common.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <QDebug>
#include <chrono>
#include <thread>

using namespace ChatUtils;

ShmClient::ShmClient(QObject* parent)
    : QObject(parent), shm_fd_(-1), shm_ptr_(nullptr), layout_(nullptr),
      mutex_sem_(nullptr), count_sem_(nullptr), joined_(false), 
      should_stop_(false), last_read_index_(0) {}

ShmClient::~ShmClient() {
    leave_room();
}

bool ShmClient::join_room(const QString& shm_name, const QString& username) {
    if (joined_) {
        emit error_occurred("Already joined");
        return false;
    }

    username_ = username;

    if (!initialize_shared_memory(shm_name)) {
        emit error_occurred("Failed to initialize shared memory");
        return false;
    }

    joined_ = true;
    should_stop_ = false;
    read_thread_ = std::thread(&ShmClient::read_loop, this);

    emit joined();
    return true;
}

void ShmClient::leave_room() {
    if (!joined_) return;

    should_stop_ = true;
    joined_ = false;

    if (read_thread_.joinable()) {
        read_thread_.join();
    }

    if (mutex_sem_) {
        sem_close(mutex_sem_);
        mutex_sem_ = nullptr;
    }
    if (count_sem_) {
        sem_close(count_sem_);
        count_sem_ = nullptr;
    }
    if (shm_ptr_ && shm_ptr_ != MAP_FAILED) {
        munmap(shm_ptr_, SHM_BUFFER_SIZE);
        shm_ptr_ = nullptr;
    }
    if (shm_fd_ >= 0) {
        close(shm_fd_);
        shm_fd_ = -1;
    }

    emit left();
}

bool ShmClient::send_message(const QString& text) {
    if (!joined_) return false;

    Message msg;
    strncpy(msg.user, username_.toStdString().c_str(), MAX_USERNAME_LEN - 1);
    strncpy(msg.timestamp, Message::get_current_timestamp().c_str(), MAX_TIMESTAMP_LEN - 1);
    strncpy(msg.text, text.toStdString().c_str(), MAX_MESSAGE_LEN - 1);

    return write_to_buffer(msg);
}

bool ShmClient::initialize_shared_memory(const QString& shm_name) {
    // Open shared memory
    shm_fd_ = shm_open(shm_name.toStdString().c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd_ < 0) {
        LOG_ERROR("ShmClient", "Failed to open shared memory");
        return false;
    }

    // Map shared memory
    shm_ptr_ = mmap(nullptr, SHM_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (shm_ptr_ == MAP_FAILED) {
        LOG_ERROR("ShmClient", "Failed to mmap shared memory");
        close(shm_fd_);
        shm_fd_ = -1;
        return false;
    }

    layout_ = static_cast<ShmLayout*>(shm_ptr_);

    // Open semaphores
    mutex_sem_ = sem_open(SHM_MUTEX_NAME, O_CREAT, 0666, 1);
    if (mutex_sem_ == SEM_FAILED) {
        LOG_ERROR("ShmClient", "Failed to open mutex semaphore");
        return false;
    }

    count_sem_ = sem_open(SHM_COUNT_NAME, O_CREAT, 0666, 0);
    if (count_sem_ == SEM_FAILED) {
        LOG_ERROR("ShmClient", "Failed to open count semaphore");
        return false;
    }

    // Initialize header on first use
 // Acquire mutex with timeout to prevent indefinite blocking
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1; // 1 second timeout
    if (sem_timedwait(mutex_sem_, &ts) != 0) {
        LOG_ERROR("ShmClient", "Timeout acquiring mutex in initialization");
        return false;
    }    if (layout_->header.capacity == 0) {
        layout_->header.read_index = 0;
        layout_->header.write_index = 0;
        layout_->header.count = 0;
        layout_->header.capacity = MAX_SLOTS;
        layout_->header.msg_size = sizeof(Message);
    }
    sem_post(mutex_sem_);

    return true;
}

bool ShmClient::write_to_buffer(const Message& msg) {
    if (!layout_) return false;

 // Acquire mutex with timeout
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1;
    if (sem_timedwait(mutex_sem_, &ts) != 0) {
        LOG_ERROR("ShmClient", "Timeout acquiring mutex in write_to_buffer");
        return false;
    }
    // Write to buffer (circular)
    int idx = layout_->header.write_index % layout_->header.capacity;
    layout_->messages[idx] = msg;
    layout_->header.write_index++;

    sem_post(mutex_sem_);
    sem_post(count_sem_);  // Signal that a message is available

    return true;
}

bool ShmClient::read_from_buffer(Message& msg) {
    if (!layout_) return false;

    // Wait for a message with timeout
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1;  // 1 second timeout

    if (sem_timedwait(count_sem_, &ts) != 0) {
        return false;  // timeout or error
    }

    sem_wait(mutex_sem_);

    int idx = layout_->header.read_index % layout_->header.capacity;
    msg = layout_->messages[idx];
    layout_->header.read_index++;

    sem_post(mutex_sem_);

    return true;
}

void ShmClient::read_loop() {
    Message msg;
    while (!should_stop_) {
        if (read_from_buffer(msg)) {
            // Don't display our own messages
            if (msg.user != username_.toStdString()) {
                emit message_received(
                    QString::fromUtf8(msg.user),
                    QString::fromUtf8(msg.timestamp),
                    QString::fromUtf8(msg.text)
                );
            }
        }
    }
}
