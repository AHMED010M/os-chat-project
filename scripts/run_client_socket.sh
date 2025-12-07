#!/bin/bash
# Start GUI client in socket mode

echo "Starting OS Chat Client (Socket Mode)..."
echo "Default: 127.0.0.1:5000"
echo ""

cd "$(dirname "$0")/../build"
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000
