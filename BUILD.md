# Build Instructions - OS Chat Project

## Quick Start

```bash
# Clone repository
git clone https://github.com/AHMED010M/os-chat-project.git
cd os_chat_project

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run tests
ctest --verbose

# Start server (Terminal 1)
./server/chat_server --port 5000

# Start client (Terminal 2)
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000 --user alice

# Start another client (Terminal 3)
./client_gui/chat_client --mode socket --ip 127.0.0.1 --port 5000 --user bob
```

---

## System Requirements

### Mandatory
- **OS:** Linux (Ubuntu 20.04 LTS or later)
- **Architecture:** x86_64 or ARM (with POSIX support)
- **Compiler:** GCC 9.0+ or Clang 10.0+ (C++17 support)
- **CMake:** 3.16 or later

### Optional
- **IDE:** VS Code, CLion, or Qt Creator
- **Debugger:** gdb or lldb

---

## Dependencies Installation

### Ubuntu/Debian

```bash
# Update package lists
sudo apt-get update

# Install build tools
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config

# Install Qt5 development
sudo apt-get install -y \
    qtbase5-dev \
    qt5-qmake \
    libqt5gui5 \
    libqt5widgets5 \
    libqt5core5a

# Optional: Qt Creator IDE
sudo apt-get install -y qt-creator

# Optional: Debugging tools
sudo apt-get install -y \
    gdb \
    lldb \
    valgrind
```

### CentOS/RHEL

```bash
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    cmake \
    qt5-qtbase-devel \
    qt5-qtdeclarative-devel
```

### macOS (Experimental)

```bash
# Install Homebrew
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

# Install dependencies
brew install cmake qt5

# Add Qt5 to CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=$(brew --prefix qt5)
```

---

## Building from Source

### Step 1: Clone Repository

```bash
git clone https://github.com/AHMED010M/os-chat-project.git
cd os_chat_project
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

```bash
# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Or Debug build (for development)
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

**Advanced Options:**
```bash
# Specify Qt5 location (if not found automatically)
cmake -DCMAKE_PREFIX_PATH=/opt/qt5 ..

# Disable tests
cmake -DBUILD_TESTING=OFF ..

# Custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
```

### Step 4: Compile

```bash
# Use all CPU cores
make -j$(nproc)

# Or specify number of jobs
make -j4

# Single-threaded (useful for debugging)
make
```

### Step 5: Install (Optional)

```bash
# Install to system directories (requires write permission)
sudo make install

# Or install to user directory
make install DESTDIR=$HOME/.local
```

---

## Build Output

After successful build, you'll find:

```
build/
├── server/
│   └── chat_server              # Socket server executable
├── client_gui/
│   └── chat_client             # GUI client executable
└── tests/
    ├── test_socket             # Socket system tests
    └── test_shm                # Shared memory tests
```

---

## Running Executables

### Socket Server

```bash
cd build
./server/chat_server --port 5000

# Output:
# [INFO] [Server] Chat server started on 0.0.0.0:5000
# [INFO] [Server] Waiting for connections... (Press Ctrl+C to stop)
```

### GUI Client (Socket Mode)

```bash
./client_gui/chat_client \
    --mode socket \
    --ip 127.0.0.1 \
    --port 5000 \
    --user alice
```

### GUI Client (Shared Memory Mode)

```bash
./client_gui/chat_client \
    --mode shm \
    --shmname /os_chat_shm \
    --user bob
```

---

## Testing

### Run All Tests

```bash
cd build
ctest --verbose
```

### Run Specific Test

```bash
# Socket tests only
./tests/test_socket

# Shared memory tests only
./tests/test_shm
```

### Test with Memory Checking

```bash
valgrind --leak-check=full \
    --show-leak-kinds=all \
    ./tests/test_socket
```

---

## Debugging

### Using GDB

```bash
# Start server under GDB
gdb ./server/chat_server
(gdb) run --port 5000
(gdb) break client_handler.cpp:50
(gdb) continue

# In another terminal, connect client to trigger breakpoint
```

### Using LLDB (Clang)

```bash
lldb ./server/chat_server
(lldb) run --port 5000
(lldb) breakpoint set -f client_handler.cpp -l 50
```

### Qt Creator IDE

```bash
# Open in Qt Creator
qt-creator CMakeLists.txt

# Or from build directory:
cd build
qt-creator ../CMakeLists.txt
```

### VS Code

```bash
# Install C++ extension
# Then open project folder:
code .
```

---

## Clean Build

```bash
# Remove build directory
rm -rf build/

# Clean shared memory artifacts
./scripts/cleanup_shm.sh

# Rebuild from scratch
mkdir build && cd build
cmake ..
make
```

---

## Troubleshooting

### CMake not found

```bash
sudo apt-get install cmake
```

### Qt5 not found

```bash
# Find Qt5 location
qt5-qmake --version
pkg-config --cflags --libs Qt5Widgets

# If not found, install:
sudo apt-get install qtbase5-dev

# Then set CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=$(pkg-config --variable=libdir Qt5Core)/..
cmake ..
```

### Compiler errors

```bash
# Ensure C++17 support
g++ --version  # Should be 9.0+

# Force C++17
cmake -DCMAKE_CXX_STANDARD=17 ..
```

### "Address already in use" on port 5000

```bash
# Kill existing process
pkill -f chat_server

# Or use different port
./server/chat_server --port 5001
```

### Shared memory permission denied

```bash
# Cleanup old segments
./scripts/cleanup_shm.sh

# Check /dev/shm permissions
ls -ld /dev/shm
# Should show: drwxrwxrwt

# If not, check with admin
```

---

## Build Variants

### Release Build (Optimized)

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Advantages: 
# - Faster execution
# - Smaller binary size
# Disadvantages:
# - Hard to debug
```

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Advantages:
# - Full debugging symbols
# - Easier to trace issues
# Disadvantages:
# - Slower execution
# - Larger binary
```

### Profile Build

```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j$(nproc)

# Best of both: Optimized + debugging symbols
```

---

## Performance Tuning

### Compiler Optimizations

```bash
cmake -DCMAKE_CXX_FLAGS_RELEASE="-O3 -march=native" ..
```

### Multi-threaded Build

```bash
make -j$(nproc)  # Use all cores
make -j8          # Or specify number
```

### Link-time Optimization

```bash
cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
```

---

## Installation

### System-wide Installation

```bash
cd build
sudo make install

# Installed to:
# /usr/local/bin/chat_server
# /usr/local/bin/chat_client
# /usr/local/share/os_chat_project/
```

### User Installation

```bash
make install DESTDIR=$HOME/.local

# Add to PATH
export PATH=$HOME/.local/bin:$PATH
```

### Creating DEB Package

```bash
cpack -G DEB

# Creates: os_chat_project-1.0-Linux.deb
sudo dpkg -i os_chat_project-1.0-Linux.deb
```

---

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install -y qtbase5-dev cmake
      - name: Build
        run: |
          mkdir build && cd build
          cmake ..
          make -j$(nproc)
      - name: Test
        run: cd build && ctest --verbose
```

---

## Build Documentation

```bash
# Generate Doxygen documentation (optional)
doxygen

# Open generated docs
firefox docs/html/index.html
```

---

**Build Guide Version:** 1.0  
**Last Updated:** 2025-12-08
