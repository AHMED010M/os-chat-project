# OS Chat Project - Architecture & Design

## Table of Contents
1. [System Overview](#system-overview)
2. [Socket System (System A)](#socket-system-system-a)
3. [Shared Memory System (System B)](#shared-memory-system-system-b)
4. [GUI Architecture](#gui-architecture)
5. [Synchronization Strategy](#synchronization-strategy)
6. [Message Protocol](#message-protocol)
7. [Thread Model](#thread-model)
8. [Extension Points](#extension-points)

---

## System Overview

```
┌─────────────────────────────────────────────────────────┐
│           Qt5 GUI Application (MainWindow)              │
│  ┌──────────────────────────────────────────────────┐   │
│  │  Mode Selector: [Socket | Shared Memory]         │   │
│  │  [Settings Panel] [Message Area] [Status]        │   │
│  └──────────────────────────────────────────────────┘   │
│                    ▲           ▲                          │
│                    │           │                          │
│        ┌───────────┘           └──────────┐              │
│        │                                   │              │
│        ▼                                   ▼              │
│  ┌──────────────┐              ┌──────────────┐           │
│  │ SocketClient │              │ ShmClient    │           │
│  │ (threading)  │              │ (threading)  │           │
│  └──────────────┘              └──────────────┘           │
└─────────────────────────────────────────────────────────┘
        │                              │
        │                              │
        ▼                              ▼
  ┌──────────────┐            ┌─────────────────┐
  │ TCP Sockets  │            │ Shared Memory   │
  │  (Network)   │            │  (Local IPC)    │
  └──────────────┘            └─────────────────┘
        │                              │
        ▼                              ▼
  ┌──────────────┐            ┌─────────────────┐
  │ chat_server  │            │ SHM Segment +   │
  │ (multithreaded)           │ POSIX Semaphores│
  └──────────────┘            └─────────────────┘
```

---

## Socket System (System A)

### Architecture

```
Client 1 (alice)   Client 2 (bob)     Client N (charlie)
       │                   │                    │
       └───────────────────┴────────────────────┘
                      │
                      │ TCP (port 5000)
                      ▼
        ┌─────────────────────────────┐
        │   chat_server               │
        │  ┌─────────────────────┐    │
        │  │ Accept Loop         │    │
        │  │ (main thread)       │    │
        │  └──────────┬──────────┘    │
        │             │               │
        │  ┌──────────┴────────────┐  │
        │  │ Thread Pool / List    │  │
        │  │ ┌────┬────┬────┐      │  │
        │  │ │ C1 │ C2 │ CN │      │  │
        │  │ └─┬──┴─┬──┴─┬──┘      │  │
        │  │   │    │    │        │  │
        │  │ ┌─┴────┴┬───┴─┐      │  │
        │  │ ▼       ▼     ▼      │  │
        │  │ Threads (1 per client)  │
        │  │ Each handles:           │
        │  │ - Receive messages      │
        │  │ - Broadcast to others   │
        │  │ - Handle disconnect     │
        │  └─────────────────────┘  │
        │                            │
        │ Protected Resources:       │
        │ - clients list (mutex)     │
        │ - broadcast queue (mutex)  │
        └─────────────────────────────┘
```

### Server Flow

1. **Initialization:**
   - Create TCP socket
   - Bind to port 5000
   - Listen for connections
   - Enable SO_REUSEADDR to avoid "Address already in use" errors

2. **Accept Loop (Main Thread):**
   - Accept new client connection
   - Create ClientHandler object
   - Start handler thread
   - Add to clients list (protected by mutex)

3. **Client Handler Thread:**
   - Receive username from client
   - Enter message receive loop
   - For each message:
     - Update timestamp
     - Broadcast to all other clients (mutex-protected)
     - Handle client disconnect gracefully

4. **Broadcasting:**
   ```cpp
   void broadcast_message(const Message& msg) {
       std::lock_guard<std::mutex> lock(clients_mutex);  // Lock
       for (auto& client : clients) {
           if (client->is_connected() && client->id != sender_id) {
               client->send_message(msg);  // Send over socket
           }
       }
   }  // Unlock
   ```

### Message Transmission (Socket)

```
Client → Server              Server → All Clients
┌─────────────┐              ┌──────────────────┐
│ Message obj │              │ Message obj      │
└──────┬──────┘              └────────┬─────────┘
       │                              │
       ▼                              ▼
  to_json()                       to_json()
  ↓                               ↓
  {"user":"alice",...}            Same message
       │                              │
       ▼                              ▼
 Length prefix (4 bytes)         Length prefix (4 bytes)
     + JSON payload                + JSON payload
       │                              │
       ▼                              ▼
  send() over socket            send() to all clients
```

---

## Shared Memory System (System B)

### Shared Memory Layout

```
┌────────────────────────────────────────────────────────┐
│                    SHM Segment                          │
│  Size: SHM_BUFFER_SIZE (1 MB default)                  │
│                                                        │
│  ┌──────────────────────────────────────────────────┐ │
│  │  ShmHeader (metadata)                            │ │
│  │  ┌────────────────────────────────────────────┐ │ │
│  │  │ read_index:  0  (consumer pointer)         │ │ │
│  │  │ write_index: 0  (producer pointer)         │ │ │
│  │  │ count:       0  (filled slots)             │ │ │
│  │  │ capacity:    64 (MAX_SLOTS)                │ │ │
│  │  │ msg_size:    sizeof(Message)               │ │ │
│  │  └────────────────────────────────────────────┘ │ │
│  └──────────────────────────────────────────────────┘ │
│                                                        │
│  ┌──────────────────────────────────────────────────┐ │
│  │  Ring Buffer: Message[MAX_SLOTS]                │ │
│  │  ┌──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┐ │
│  │  │0 │1 │2 │3 │4 │5 │6 │7 │..│62│63│  │  │  │ │  Circular
│  │  └──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┘ │  wrapping
│  │  ▲                                            ▲    │
│  │  │ read_index                   write_index  │    │
│  │  └────────────────────────────────────────────┘    │
│  │                                                    │
│  │  Each slot contains:                              │
│  │  struct Message {                                 │
│  │      char user[32];                               │
│  │      char timestamp[32];                          │
│  │      char text[512];                              │
│  │  };                                               │
│  └──────────────────────────────────────────────────┘ │
└────────────────────────────────────────────────────────┘
```

### Synchronization Mechanism

#### POSIX Semaphores

```
┌──────────────────────────────────────────────────────┐
│  POSIX Named Semaphores (in /dev/shm)                │
│                                                      │
│  1. /os_chat_mutex                                   │
│     Type: Binary semaphore (sem_t)                   │
│     Initial value: 1                                 │
│     Purpose: Protect read_index & write_index        │
│                                                      │
│  2. /os_chat_count                                   │
│     Type: Counting semaphore (sem_t)                 │
│     Initial value: 0                                 │
│     Purpose: Track number of unread messages         │
│                                                      │
│  Usage Pattern:                                      │
│  - Writer: sem_wait(mutex), write, sem_post(count)   │
│  - Reader: sem_wait(count), sem_wait(mutex), read    │
└──────────────────────────────────────────────────────┘
```

### Producer-Consumer Protocol

```
┌─────────────────────────┐      ┌─────────────────────────┐
│   Producer (Writer)     │      │   Consumer (Reader)     │
│                         │      │                         │
│ 1. sem_wait(mutex)      │      │ 1. sem_wait(count)      │
│    [LOCK]               │      │    [BLOCK if empty]     │
│                         │      │                         │
│ 2. Compute slot index   │      │ 2. sem_wait(mutex)      │
│    slot = write++ % cap │      │    [LOCK]               │
│                         │      │                         │
│ 3. Write message        │      │ 3. Compute slot index   │
│    buffer[slot] = msg   │      │    slot = read++ % cap  │
│                         │      │                         │
│ 4. Update count         │      │ 4. Read message         │
│    count++              │      │    msg = buffer[slot]   │
│                         │      │                         │
│ 5. sem_post(mutex)      │      │ 5. sem_post(mutex)      │
│    [UNLOCK]             │      │    [UNLOCK]             │
│                         │      │                         │
│ 6. sem_post(count)      │      │                         │
│    [SIGNAL]             │      │ 6. (goto 1)             │
└─────────────────────────┘      └─────────────────────────┘
```

### Ring Buffer Index Management

```
Capacity = 4 (simplified example)

Initial state:
read_idx=0, write_idx=0
┌─┬─┬─┬─┐
│ │ │ │ │
└─┴─┴─┴─┘

After 4 writes:
read_idx=0, write_idx=4
┌─┬─┬─┬─┐
│A│B│C│D│
└─┴─┴─┴─┘
     (full)

After 2 reads:
read_idx=2, write_idx=4
┌─┬─┬─┬─┐
│A│B│C│D│
└─┴─┴─┴─┘
   ^   ^
   r   w

After 3 more writes:
read_idx=2, write_idx=7
┌─┬─┬─┬─┐
│E│B│F│G│  ← Note: E overwrites A (7 % 4 = 3)
└─┴─┴─┴─┘     F overwrites B (6 % 4 = 2)
   ^   ^      G overwrites C (5 % 4 = 1)
   r   w

Indices never reset, they wrap through modulo
```

---

## GUI Architecture

### MainWindow Class Diagram

```
┌──────────────────────────────────┐
│        MainWindow (Qt5)           │
├──────────────────────────────────┤
│ - mode_combo_ : QComboBox        │
│ - ip_input_ : QLineEdit          │
│ - port_input_ : QSpinBox         │
│ - username_input_ : QLineEdit    │
│ - messages_display_ : QTextEdit  │
│ - message_input_ : QLineEdit     │
│ - socket_client_ : SocketClient* │
│ - shm_client_ : ShmClient*       │
│ - is_connected_ : bool           │
├──────────────────────────────────┤
│ + setup_ui() : void              │
│ + update_ui_for_mode() : void    │
│ + on_connect_button_clicked()    │
│ + on_send_button_clicked()       │
│ + on_message_received()          │
│ + append_message()               │
└──────────────────────────────────┘
```

### Signal/Slot Flow

```
[Backend Thread]              [GUI Thread]
       │                            │
       │ message_received(u,t,x)    │
       ├───────────────────────────>│
       │                            ▼
       │                    append_message()
       │                            │
       │                            ▼
       │                    Update QTextEdit
       │
       ├─ connected()
       │                            ▼
       │                    on_connected()
       │                            ▼
       │                    Update UI state
       │                    (enable inputs)
       │
       └─ disconnected()
                                    ▼
                            on_disconnected()
                                    ▼
                            Update UI state
                            (disable inputs)
```

---

## Synchronization Strategy

### Socket System

**Shared Resources:**
- `clients` vector
- Broadcast queue
- Message forwarding

**Protection Mechanism:**
```cpp
std::mutex clients_mutex;
std::vector<ClientHandler*> clients;

void broadcast_message(const Message& msg) {
    std::lock_guard<std::mutex> lock(clients_mutex);  // RAII lock
    for (auto client : clients) {
        if (client->is_connected()) {
            client->send_message(msg);  // Thread-safe send
        }
    }
}  // Lock automatically released here
```

**Why this works:**
- `std::lock_guard` ensures mutex is released even if exception thrown
- Each client has its own thread, minimal contention
- Broadcast time is brief (just iterate and send)

### Shared Memory System

**Shared Resources:**
- Ring buffer (Message[MAX_SLOTS])
- read_index
- write_index
- count

**Protection Mechanism:**
```cpp
sem_t *mutex = sem_open("/os_chat_mutex", O_CREAT, 0666, 1);
sem_t *count = sem_open("/os_chat_count", O_CREAT, 0666, 0);

void write_message(Message msg) {
    sem_wait(mutex);           // Acquire lock
    int idx = write_idx++ % MAX_SLOTS;
    messages[idx] = msg;       // Critical section
    count_++;                  // Update counter
    sem_post(mutex);           // Release lock
    sem_post(count);           // Signal waiting readers
}

void read_message(Message &msg) {
    sem_wait(count);           // Block if no messages
    sem_wait(mutex);           // Acquire lock
    int idx = read_idx++ % MAX_SLOTS;
    msg = messages[idx];       // Critical section
    sem_post(mutex);           // Release lock
    // No post(count) here - reader consumed one
}
```

**Why this works:**
- Binary semaphore (mutex) ensures mutual exclusion
- Counting semaphore (count) tracks available messages
- Reader blocks on empty buffer automatically
- No busy-waiting, efficient use of system resources

---

## Message Protocol

### JSON Format

```json
{"user":"alice","time":"2025-12-08T01:47:00Z","text":"Hello, world!"}
```

**Fields:**
- `user` (string): Username of sender (max 32 chars)
- `time` (ISO 8601): Timestamp in UTC (max 32 chars)
- `text` (string): Message content (max 512 chars)

### Transmission over Socket

```
Binary Format:
┌──────────────────────────────────────────┐
│  4 bytes (big-endian uint32_t)           │ Length
│  ┌──────────────────────────────────────┐│
│  │ {"user":...}\n                       ││ JSON payload
│  └──────────────────────────────────────┘│
└──────────────────────────────────────────┘

Example: Length=63, Payload=(JSON + newline)
00 00 00 3F {"user":"alice","time":"...","text":"Hi"}\n
```

### Transmission in Shared Memory

```
Fixed-size slots in ring buffer:
┌────────────────────────────────────┐
│  struct Message                    │
│  {                                 │
│      char user[32];      // 32B    │
│      char timestamp[32]; // 32B    │
│      char text[512];     // 512B   │
│  };                      // 576B   │
└────────────────────────────────────┘

No length prefix needed (fixed size)
Each slot is always 576 bytes
```

---

## Thread Model

### Socket Server

```
Main Process
│
├─ Thread 0: Accept Loop
│  - Listens on socket
│  - Accepts new connections
│  - Creates handler threads
│  - Registers clients
│
├─ Thread 1: ClientHandler (Client 1 - alice)
│  - Reads from Client 1's socket
│  - Broadcasts messages to other clients
│  - Handles disconnect
│
├─ Thread 2: ClientHandler (Client 2 - bob)
│  - Reads from Client 2's socket
│  - Broadcasts messages to other clients
│  - Handles disconnect
│
└─ Thread N: ClientHandler (Client N)
   - (similar)

All threads share:
- clients_mutex (protects vector)
- clients list
- broadcast queue
```

### GUI Client

```
Main Process (Qt)
│
├─ Thread 0: Qt Event Loop
│  - Handles UI events (button clicks, input)
│  - Updates display when signals received
│  - NOT blocked on I/O
│
├─ Thread 1: SocketClient::receive_loop() [if in socket mode]
│  - Blocked on socket recv()
│  - Emits message_received() signal
│  - Runs in background, doesn't block UI
│
└─ Thread 2: ShmClient::read_loop() [if in shm mode]
   - Blocked on sem_wait(count)
   - Emits message_received() signal
   - Runs in background, doesn't block UI

UI updates happen via Qt signals (thread-safe)
```

---

## Extension Points

### Adding Features

1. **User List (Socket Mode)**
   - Add `online_users` set to server
   - Send user list on connect
   - Broadcast join/leave events

2. **Message History**
   - Write messages to SQLite database
   - Load history on connect
   - Display in separate tab

3. **File Sharing**
   - Extend protocol with file chunks
   - Use separate data channel
   - Progress bar in GUI

4. **Encryption**
   - Use OpenSSL for TLS
   - Encrypt SHM messages with shared key
   - Add key exchange protocol

5. **Scalability (Socket Mode)**
   - Replace thread-per-client with thread pool
   - Use epoll/kqueue for I/O multiplexing
   - Implement message queue

6. **Performance (SHM Mode)**
   - Use spin locks instead of semaphores
   - Implement lock-free ring buffer
   - Add read-ahead buffering

---

**Document Version:** 1.0  
**Last Updated:** 2025-12-08
