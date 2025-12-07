# OS Chat Project: Multi-Threaded System with Socket & Shared Memory

> **A complete C++ project demonstrating operating system concepts:** multithreading, socket programming, IPC using POSIX shared memory, synchronization primitives (semaphores), and GUI integration with Qt5.

## 📋 Project Overview

This project implements a **unified chat application** with **two separate backends**:

1. **System A – Network Chat (BSD Sockets)**
   - Remote clients connect via TCP sockets
   - Server accepts multiple concurrent clients using multithreading
   - Messages broadcast to all connected users
   - Clients can be on different machines

2. **System B – Local Chat (POSIX Shared Memory)**
   - Multiple processes on the same machine communicate via shared memory
   - Ring buffer of message slots protected by POSIX semaphores
   - Zero-copy message passing between processes
   - Lightweight, low-latency communication

Both systems use the **same Qt5 GUI** – just toggle the mode!

---

## ✨ Features

### Socket System (System A)
- ✅ Multi-threaded server (thread-per-client or pool-based)
- ✅ Username registration and online user tracking
- ✅ Message broadcasting with JSON protocol
- ✅ Graceful client disconnect and server shutdown
- ✅ Configurable port (default: 5000)

### Shared Memory System (System B)
- ✅ POSIX shared memory (`shm_open` + `mmap`)
- ✅ POSIX named semaphores for synchronization
- ✅ Ring buffer with fixed message slots (64 slots default)
- ✅ Per-message metadata: username, timestamp, text (max 512 bytes)
- ✅ Multi-process producer-consumer without race conditions

### GUI Application
- ✅ Qt5 Widgets interface
- ✅ Message display with timestamps and usernames
- ✅ Input field and send button
- ✅ Mode selector: Socket or Shared Memory
- ✅ Server IP:port configuration (for socket mode)
- ✅ Online users list (socket mode only)
- ✅ Connection status indicator
- ✅ Auto-scrolling message view

---

## 🛠️ Prerequisites

### System Requirements
- **OS:** Linux (Ubuntu 20.04/22.04 or similar)
- **Architecture:** x86_64 or ARM (with POSIX support)

### Build Dependencies
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    g++ \
    qt5-qmake \
    qt5-default \
    libqt5gui5 \
    libqt5widgets5 \
    libqt5core5a \
    qtbase5-dev \
    git
```

---

## 📦 Project Structure

```
os_chat_project/
├── CMakeLists.txt                 # Top-level build configuration
├── README.md                       # This file
├── LICENSE                         # MIT License
│
├── server/                         # Socket server (System A)
│   ├── CMakeLists.txt
│   ├── server.cpp                 # Main server loop & client threads
│   ├── client_handler.h           # ClientHandler class declaration
│   └── client_handler.cpp         # Client connection management
│
├── client_gui/                     # Qt5 GUI application
│   ├── CMakeLists.txt
│   ├── main.cpp                   # Application entry point
│   ├── MainWindow.h               # GUI window declaration
│   ├── MainWindow.cpp             # GUI implementation & event handlers
│   ├── SocketClient.h             # Socket communication class
│   ├── SocketClient.cpp           # Socket implementation
│   ├── ShmClient.h                # Shared memory client class
│   └── ShmClient.cpp              # Shared memory implementation
│
├── shared/                         # Shared headers & structures
│   ├── common.h                   # Message format, shared memory layout
│   └── protocol.h                 # Protocol definitions & constants
│
├── tests/                          # Automated tests
│   ├── CMakeLists.txt
│   ├── test_socket.cpp            # Socket system tests
│   └── test_shm.cpp               # Shared memory system tests
│
├── scripts/                        # Utility scripts
│   ├── run_server.sh              # Start socket server
│   ├── run_client_socket.sh       # Start GUI in socket mode
│   ├── run_client_shm.sh          # Start GUI in shared memory mode
│   └── cleanup_shm.sh             # Clean up shared memory artifacts
│
├── docs/                           # Documentation
│   ├── architecture.md            # System design & synchronization
│   ├── testing.md                 # Test procedures
│   ├── video_script.md            # Demo script for video recording
│   └── screenshots/               # Demo screenshots
│       ├── screenshot1.png        # Socket system - two clients
│       ├── screenshot2.png        # Message exchange
│       └── screenshot3.png        # Shared memory mode
│
└── demo_data/
    └── sample_commands.txt        # Example usage commands
```

---

## 🚀 Quick Start

### 1. Clone & Build

```bash
git clone https://github.com/AHMED010M/os-chat-project.git
cd os_chat_project

mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 2. Run Socket System (Default)

**Terminal 1 – Start Server:**
```bash
./server/chat_server --port 5000
```

**Terminal 2 – Client 1:**
```bash
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000 --user alice
```

**Terminal 3 – Client 2:**
```bash
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000 --user bob
```

### 3. Run Shared Memory System

```bash
./scripts/cleanup_shm.sh

# Terminal 1
./client_gui/chat_client --mode shm --shmname /os_chat_shm --user alice

# Terminal 2
./client_gui/chat_client --mode shm --shmname /os_chat_shm --user bob
```

---

## 📚 Documentation

See **[docs/architecture.md](docs/architecture.md)** for:
- System architecture and design
- Socket server threading model
- Shared memory layout
- Synchronization strategy
- Message protocol

See **[docs/video_script.md](docs/video_script.md)** for demo instructions.

---

## 📝 Message Format

JSON objects (newline-terminated):
```json
{"user":"alice","time":"2025-12-08T01:47:00Z","text":"Hello!"}
```

---

## 🔧 Troubleshooting

| Problem | Solution |
|---------|----------|
| Address already in use | `pkill -f chat_server` |
| Permission denied (shm_open) | Run `cleanup_shm.sh` |
| Qt5 not found | `sudo apt-get install qtbase5-dev` |

---

## 📄 License

MIT License – See [LICENSE](LICENSE) file.

---

**Happy chatting! 💬**
