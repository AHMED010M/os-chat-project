#!/bin/bash
# Start the socket chat server

echo "Starting OS Chat Server..."
echo "Listening on port 5000"
echo "Press Ctrl+C to stop"
echo ""

cd "$(dirname "$0")/../build"
./server/chat_server --port 5000
