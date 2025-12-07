# Testing Guide - OS Chat Project

## Overview

This document describes how to run and validate both the Socket System and Shared Memory System.

---

## Unit Tests

### Building Tests

```bash
cd ~/os_chat_project
mkdir build && cd build
cmake ..
make -j4
```

### Running Socket System Tests

```bash
./tests/test_socket
```

**Expected Output:**
```
========== Socket System Tests ==========

=== Test: Message Protocol ===
JSON: {"user":"alice","time":"2025-12-08T01:47:00Z","text":"Hello, world!"}
✓ Message protocol test passed

=== Test: Timestamp Generation ===
Timestamp: 2025-12-08T01:47:25Z
✓ Timestamp test passed

=== Test: Socket Communication ===
[INFO] [Server] Chat server started on 0.0.0.0:15000
[INFO] [ClientHandler] Client 0 connected as "test_user"
✓ Socket communication test passed

========== All Tests Passed! ==========
```

### Running Shared Memory Tests

```bash
./tests/test_shm
```

**Expected Output:**
```
========== Shared Memory System Tests ==========

=== Test: Message Structure ===
✓ Message structure test passed

=== Test: Semaphore Creation & Cleanup ===
✓ Semaphore test passed

=== Test: Shared Memory Creation & Access ===
✓ Shared memory test passed

=== Test: Ring Buffer Logic ===
Write index: 16, Read index: 8
✓ Ring buffer test passed

=== Test: Producer-Consumer with Semaphores ===
✓ Producer-consumer test passed

========== All Tests Passed! ==========
```

### Running All Tests at Once

```bash
cd build
ctest --verbose
```

---

## Integration Tests (Manual)

### Test 1: Socket Mode with Two Clients

#### Setup

```bash
cd ~/os_chat_project/build
```

#### Terminal 1: Start Server

```bash
./server/chat_server --port 5000
```

**Expected Output:**
```
[INFO] [Server] Chat server started on 0.0.0.0:5000
[INFO] [Server] Waiting for connections... (Press Ctrl+C to stop)
```

#### Terminal 2: Start Client 1 (alice)

```bash
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000 --user alice
```

The GUI window should appear with:
- Mode: "Socket (Network)" selected
- IP: 127.0.0.1, Port: 5000
- Username: alice
- Connection status: "Connected" (green)

#### Terminal 3: Start Client 2 (bob)

```bash
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000 --user bob
```

Same as above but with username: bob

#### Server Output After Connections

```
[INFO] [Server] New connection from 127.0.0.1:54234
[INFO] [ClientHandler] Client 0 connected as "alice"
[INFO] [Server] New connection from 127.0.0.1:54235
[INFO] [ClientHandler] Client 1 connected as "bob"
```

#### Test Messages

**In alice's client:**
- Type: "Hello, bob!"
- Click Send
- Expected: Message appears in both alice's and bob's message area

**In bob's client:**
- Type: "Hi alice! How are you?"
- Click Send
- Expected: Message appears in both clients' message areas

**Verification Checklist:**
- [ ] Messages appear immediately in sender's window
- [ ] Messages appear in other clients' windows
- [ ] Timestamps are accurate
- [ ] Usernames are correct
- [ ] Can send multiple messages
- [ ] No latency issues

---

### Test 2: Shared Memory Mode with Two Clients

#### Setup

```bash
cd ~/os_chat_project/build

# Clean up any old SHM artifacts
../scripts/cleanup_shm.sh
```

#### Terminal 1: Start Client 1 (alice)

```bash
./client_gui/chat_client --mode shm --shmname /os_chat_shm --user alice
```

GUI should show:
- Mode: "Shared Memory (Local)" selected
- SHM Name: /os_chat_shm
- Username: alice
- Connection status: "Joined" (green)

#### Terminal 2: Start Client 2 (bob)

```bash
./client_gui/chat_client --mode shm --shmname /os_chat_shm --user bob
```

Same as above but with username: bob

#### Test Messages

**In alice's client:**
- Type: "Hello from shared memory!"
- Click Send
- Expected: Appears immediately

**In bob's client:**
- Type: "Received! No network needed."
- Click Send
- Expected: Appears in both windows

**Observations:**
- Latency should be very low (local IPC)
- Messages stored in shared ring buffer
- Each client sees messages from other clients

**Verification Checklist:**
- [ ] Both clients connect successfully
- [ ] Messages appear with correct usernames
- [ ] Timestamps are accurate
- [ ] No network traffic (use `netstat` to verify)
- [ ] Very low latency

---

### Test 3: Mode Switching

#### Procedure

1. Start in Socket mode with two clients
2. Send a few messages
3. Switch mode combo box to "Shared Memory"
   - Expected: Connection status changes to "Disconnected"
   - Expected: Input field is disabled
4. Connect to SHM
   - Expected: Status changes to "Joined"
   - Expected: Input field is enabled
5. Send messages in SHM mode
6. Switch back to Socket mode
   - Expected: Reconnects to socket server
   - Expected: Can send messages again

**Verification Checklist:**
- [ ] Mode switch disconnects previous connection
- [ ] Can seamlessly switch between modes
- [ ] UI updates correctly for each mode
- [ ] Both modes work independently

---

### Test 4: Disconnection Handling

#### Socket Mode: Kill Server

1. Start server and two clients
2. Verify messages work
3. In server terminal: Press Ctrl+C
   - Expected: Clients show "Disconnected" status
   - Expected: Input field becomes disabled
   - Expected: Error message appears in message area
4. Can reconnect by clicking "Connect" button

#### Shared Memory Mode: Cleanup

1. Start two SHM clients
2. Verify messages work
3. In another terminal: Run `./scripts/cleanup_shm.sh`
   - Expected: Both clients lose connection (next recv fails)
   - Expected: Status changes to "Disconnected"
4. Can rejoin by clicking "Connect" button (new SHM segment created)

**Verification Checklist:**
- [ ] Graceful disconnection handling
- [ ] No crashes on disconnect
- [ ] Can reconnect after disconnect
- [ ] Error messages are clear

---

### Test 5: Stress Test (Socket Mode)

#### Rapid Messaging

```bash
# In one client terminal, send 100 messages rapidly
for i in {1..100}; do
  echo "Message $i"
done
```

Or manually:
1. Type and send message
2. Immediately send another
3. Send 5-10 messages rapidly

**Expected:**
- All messages appear in order
- No message loss
- No duplicate messages
- No UI freezing

#### Verification Checklist:
- [ ] All messages received
- [ ] Correct ordering
- [ ] No message corruption
- [ ] UI remains responsive

---

### Test 6: Large Message Handling

#### Socket Mode

1. In one client, type a very long message (> 500 chars)
2. Send it
3. Expected: Message truncated to MAX_MESSAGE_LEN (512 chars)
4. Other client receives truncated message

#### Shared Memory Mode

1. Same as above
2. Expected: Same truncation behavior

**Verification Checklist:**
- [ ] Long messages don't crash system
- [ ] Messages are truncated safely
- [ ] No buffer overflow

---

## Performance Tests

### Measure Latency

#### Socket Mode

```bash
# In Client 1
echo "Starting test at $(date +%s%N)" && echo "test" > /tmp/send_time.txt

# Client 2 receives and records
echo "Received at $(date +%s%N)"

# Calculate: diff = recv_time - send_time
```

**Expected:** < 100 ms for local network

#### Shared Memory Mode

```bash
# Same procedure
# Expected: < 10 ms (local IPC is much faster)
```

### Message Throughput

```bash
# Measure how many messages per second
time (
  for i in {1..1000}; do
    echo "Message $i" | ./client_gui/chat_client ...
  done
)
```

**Expected:**
- Socket mode: 100-500 msgs/sec
- Shared Memory mode: 1000+ msgs/sec

---

## Debugging

### Enable Verbose Logging

In `shared/common.h`, uncomment debug lines:

```cpp
#define ENABLE_DEBUG_LOGGING
```

### Check Shared Memory Status

```bash
# List shared memory segments
ls -la /dev/shm/ | grep os_chat

# Check semaphores
ls -la /dev/shm/sem.* | grep os_chat

# Check file descriptors
lsof | grep os_chat_shm
```

### Monitor Network Traffic

```bash
# Watch socket connections
watch -n 1 'netstat -an | grep 5000'

# Or use tcpdump
sudo tcpdump -i lo port 5000
```

### Check Thread Status

```bash
# View threads of server
ps -eLf | grep chat_server

# View thread-local variables
gdb -p PID
(gdb) info threads
```

---

## Troubleshooting

### "Address already in use" Error

```bash
pkill -f chat_server
# Wait a few seconds for TIME_WAIT to expire
sleep 10
# Try again
```

Or use SO_REUSEADDR (already implemented).

### Shared Memory Permission Denied

```bash
./scripts/cleanup_shm.sh
# Check /dev/shm permissions
ls -ld /dev/shm
# Should be: drwxrwxrwt
```

### Qt5 Not Found

```bash
sudo apt-get install qtbase5-dev
pkg-config --cflags --libs Qt5Widgets
```

### Connection Timeout

1. Verify server is running: `netstat -an | grep 5000`
2. Check firewall: `sudo ufw status`
3. Try localhost first, then IP addresses

### Messages Not Appearing

1. Check both clients are connected (status = green)
2. Check message_input_ is not empty
3. Verify send button clicked (not just typed)
4. Check for error messages in output

---

## Automated Test Script

```bash
#!/bin/bash
# run_all_tests.sh

echo "Running OS Chat Project Test Suite"
echo "===================================="

cd build || exit 1

echo "\n1. Running Socket Tests..."
./tests/test_socket || exit 1

echo "\n2. Running Shared Memory Tests..."
./tests/test_shm || exit 1

echo "\n3. Cleanup and Summary"
echo "All automated tests passed!"
```

---

## Test Coverage

| Component | Tested | Status |
|-----------|--------|--------|
| Message protocol | Unit test | ✓ |
| Socket communication | Unit test + Integration | ✓ |
| SHM layout | Unit test | ✓ |
| Semaphores | Unit test | ✓ |
| Ring buffer | Unit test | ✓ |
| Producer-consumer | Unit test | ✓ |
| GUI mode switching | Manual integration | ✓ |
| Disconnection handling | Manual integration | ✓ |
| Multi-client messaging | Manual integration | ✓ |
| Performance | Manual benchmark | ✓ |

---

**Testing Guide Version:** 1.0  
**Last Updated:** 2025-12-08
