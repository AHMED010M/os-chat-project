#!/bin/bash
# Clean up POSIX shared memory and semaphore artifacts

echo "Cleaning up shared memory and semaphore artifacts..."

# Remove shared memory segments
rm -f /dev/shm/os_chat_shm
rm -f /dev/shm/sem.os_chat_mutex
rm -f /dev/shm/sem.os_chat_count

echo "Cleanup complete."
echo "Note: If using sem_unlink(), those semaphores are now deallocated."
