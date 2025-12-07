/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Shared Memory System Tests
 */

#include <iostream>
#include <cassert>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <thread>
#include <chrono>
#include "../shared/protocol.h"

void test_message_struct() {
    std::cout << "\n=== Test: Message Structure ===" << std::endl;

    Message msg;
    strncpy(msg.user, "bob", MAX_USERNAME_LEN - 1);
    strncpy(msg.timestamp, "2025-12-08T01:47:00Z", MAX_TIMESTAMP_LEN - 1);
    strncpy(msg.text, "SHM test", MAX_MESSAGE_LEN - 1);

    assert(msg.user[0] == 'b');
    assert(strlen(msg.timestamp) > 0);
    assert(strcmp(msg.text, "SHM test") == 0);

    std::cout << "✓ Message structure test passed" << std::endl;
}

void test_semaphore_creation() {
    std::cout << "\n=== Test: Semaphore Creation & Cleanup ===" << std::endl;

    const char* test_sem = "/test_os_chat_sem";

    // Remove old semaphore if exists
    sem_unlink(test_sem);

    // Create semaphore
    sem_t* sem = sem_open(test_sem, O_CREAT, 0666, 1);
    assert(sem != SEM_FAILED);

    // Wait and post
    assert(sem_wait(sem) == 0);
    assert(sem_post(sem) == 0);

    // Close and unlink
    assert(sem_close(sem) == 0);
    assert(sem_unlink(test_sem) == 0);

    std::cout << "✓ Semaphore test passed" << std::endl;
}

void test_shared_memory_creation() {
    std::cout << "\n=== Test: Shared Memory Creation & Access ===" << std::endl;

    const char* shm_name = "/test_os_chat_shm";

    // Clean up old segment
    shm_unlink(shm_name);

    // Create shared memory
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    assert(shm_fd >= 0);

    // Set size
    size_t size = sizeof(ShmLayout);
    assert(ftruncate(shm_fd, size) == 0);

    // Map memory
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    assert(ptr != MAP_FAILED);

    // Access as ShmLayout
    ShmLayout* layout = static_cast<ShmLayout*>(ptr);
    layout->header.read_index = 0;
    layout->header.write_index = 0;
    layout->header.capacity = MAX_SLOTS;

    // Verify write
    assert(layout->header.capacity == MAX_SLOTS);

    // Cleanup
    assert(munmap(ptr, size) == 0);
    assert(close(shm_fd) == 0);
    assert(shm_unlink(shm_name) == 0);

    std::cout << "✓ Shared memory test passed" << std::endl;
}

void test_ring_buffer_logic() {
    std::cout << "\n=== Test: Ring Buffer Logic ===" << std::endl;

    const int capacity = 8;
    int write_idx = 0;
    int read_idx = 0;

    // Simulate writes
    for (int i = 0; i < capacity * 2; ++i) {
        int slot = write_idx % capacity;
        assert(slot >= 0 && slot < capacity);
        write_idx++;
    }

    // Simulate reads
    read_idx = 0;
    for (int i = 0; i < capacity; ++i) {
        int slot = read_idx % capacity;
        assert(slot >= 0 && slot < capacity);
        read_idx++;
    }

    std::cout << "Write index: " << write_idx << ", Read index: " << read_idx << std::endl;
    assert(write_idx - read_idx == capacity);

    std::cout << "✓ Ring buffer test passed" << std::endl;
}

void test_producer_consumer() {
    std::cout << "\n=== Test: Producer-Consumer with Semaphores ===" << std::endl;

    const char* mutex_sem_name = "/test_mutex_shm";
    const char* count_sem_name = "/test_count_shm";

    sem_unlink(mutex_sem_name);
    sem_unlink(count_sem_name);

    sem_t* mutex = sem_open(mutex_sem_name, O_CREAT, 0666, 1);
    sem_t* count = sem_open(count_sem_name, O_CREAT, 0666, 0);

    assert(mutex != SEM_FAILED);
    assert(count != SEM_FAILED);

    int shared_value = 0;
    bool producer_ran = false;
    bool consumer_ran = false;

    // Producer thread
    std::thread producer([&]() {
        sem_wait(mutex);
        shared_value = 42;
        sem_post(mutex);
        sem_post(count);
        producer_ran = true;
    });

    // Consumer thread
    std::thread consumer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sem_wait(count);
        sem_wait(mutex);
        assert(shared_value == 42);
        sem_post(mutex);
        consumer_ran = true;
    });

    producer.join();
    consumer.join();

    assert(producer_ran && consumer_ran);

    sem_close(mutex);
    sem_close(count);
    sem_unlink(mutex_sem_name);
    sem_unlink(count_sem_name);

    std::cout << "✓ Producer-consumer test passed" << std::endl;
}

int main() {
    std::cout << "\n========== Shared Memory System Tests ==========\n" << std::endl;

    try {
        test_message_struct();
        test_semaphore_creation();
        test_shared_memory_creation();
        test_ring_buffer_logic();
        test_producer_consumer();

        std::cout << "\n========== All Tests Passed! ==========\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed: " << e.what() << std::endl;
        return 1;
    }
}
