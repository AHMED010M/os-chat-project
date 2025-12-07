# Quick Start Guide

## 5-Minute Setup

### 1. Install Dependencies (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake qtbase5-dev git
```

### 2. Clone & Build
```bash
git clone https://github.com/AHMED010M/os-chat-project.git
cd os_chat_project
mkdir build && cd build
cmake .. && make -j$(nproc)
```

### 3. Run Demo

**Terminal 1: Start Server**
```bash
cd build
./server/chat_server --port 5000
```

**Terminal 2: Client 1**
```bash
cd build
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000 --user alice
```

**Terminal 3: Client 2**
```bash
cd build
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000 --user bob
```

**Now**: Type messages in either GUI and hit Send. They appear in both clients instantly!

---

## Shared Memory Mode (No Server)

**Terminal 1:**
```bash
cd build
./client_gui/chat_client --mode shm --shmname /os_chat_shm --user alice
```

**Terminal 2:**
```bash
cd build
./client_gui/chat_client --mode shm --shmname /os_chat_shm --user bob
```

**Type messages** – they appear instantly with ZERO network latency!

---

## What You're Looking At

🔗 **Two Chat Systems in One App:**
- **Socket Mode**: Traditional network chat (clients on different machines)
- **Shared Memory Mode**: Ultra-fast local IPC (same machine, < 1ms latency)

📈 **Real OS Concepts Demonstrated:**
- Multi-threaded server (pthread)
- TCP socket programming (BSD sockets)
- POSIX shared memory (shm_open, mmap)
- Semaphore synchronization (sem_t)
- Ring buffer data structure
- Qt5 GUI with signals/slots
- Thread-safe design (mutex protection)

---

## Project Structure
```
os_chat_project/
│
├─ server/          → Multi-threaded TCP server
├─ client_gui/       → Qt5 GUI (works with both backends)
├─ shared/          → Protocol & common code
├─ tests/           → Automated tests
├─ scripts/         → Utility shell scripts
├─ docs/            → Full documentation
└─ CMakeLists.txt   → Build configuration
```

---

## Key Features

✅ **Socket System (System A)**
- Multi-threaded server handles multiple clients
- Message broadcast via TCP
- Works across networks
- Configurable port
- Graceful shutdown

✅ **Shared Memory System (System B)**
- Ring buffer (64-slot capacity)
- POSIX semaphores for sync
- Zero-copy message passing
- Ultra-low latency
- Local machine only

✅ **GUI (Works with Both)**
- Mode selector (Socket/SHM toggle)
- Real-time message display
- User list (socket mode)
- Connection status indicator
- Input validation

✅ **Production Quality**
- C++17 with modern patterns
- Thread-safe design
- Comprehensive error handling
- Full test coverage
- MIT Licensed

---

## Testing

```bash
cd build
ctest --verbose
```

Runs:
- Protocol tests (JSON serialization)
- Socket communication tests  
- Shared memory tests
- Semaphore tests
- Ring buffer tests
- Producer-consumer tests

---

## Architecture

```
Socket Mode:              Shared Memory Mode:

Server                    Process 1 (alice)
 │                        │
├───────               ├───────
Thread 1 (alice)          │
Thread 2 (bob)            ├── Shared Memory
│                        │  Ring Buffer
Broadcast                 │  + Semaphores
 │                        │
Client 1 │ Client 2      Process 2 (bob)
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "Address already in use" | `pkill -f chat_server` then retry |
| Qt5 not found | `sudo apt-get install qtbase5-dev` |
| SHM permission denied | `./scripts/cleanup_shm.sh` |
| Build fails | `rm -rf build && mkdir build && cd build && cmake .. && make` |

---

## Next Steps

1. **Explore Code**: Read `shared/protocol.h` to see message format
2. **Study Architecture**: See `docs/architecture.md` for detailed design
3. **Run Tests**: `ctest --verbose` to see all tests pass
4. **Record Video**: Follow `docs/video_script.md` for demo steps
5. **Extend**: Add features like file sharing, encryption, etc.

---

## Documentation

- **README.md** - Full project overview
- **BUILD.md** - Detailed build instructions
- **docs/architecture.md** - System design & synchronization
- **docs/testing.md** - Testing procedures
- **docs/video_script.md** - Demo recording steps

---

## License

MIT License – Free to use and modify

---

**Made with ♥ for learning operating systems concepts**
