#!/bin/bash
# Start GUI client in shared memory mode

echo "Starting OS Chat Client (Shared Memory Mode)..."
echo "Using shared memory: /os_chat_shm"
echo ""

cd "$(dirname "$0")/../build"
./client_gui/chat_client --mode shm --shmname /os_chat_shm
