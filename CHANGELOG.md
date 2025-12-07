# Changelog

All notable changes to OS Chat Project will be documented here.

## [1.0.0] - 2025-12-08

### Added
- Initial release with dual-backend chat system
- **Socket System (System A)**
  - Multi-threaded TCP server
  - Per-client handler threads
  - Message broadcasting
  - Graceful shutdown
  - Configurable port (default 5000)

- **Shared Memory System (System B)**
  - POSIX shared memory segments
  - Ring buffer (64-slot capacity)
  - POSIX named semaphores
  - Producer-consumer synchronization
  - Zero-copy message passing

- **Qt5 GUI Client**
  - Unified interface for both backends
  - Mode selector (Socket/Shared Memory)
  - Real-time message display
  - User status indicator
  - Connection controls
  - Message timestamp display

- **Core Features**
  - JSON message protocol
  - Automatic timestamp generation (ISO 8601)
  - Thread-safe design
  - Signal/slot communication (GUI)
  - Proper resource cleanup

- **Testing**
  - Unit tests for socket system
  - Unit tests for shared memory system
  - Integration test infrastructure
  - Performance benchmarking support

- **Documentation**
  - Comprehensive README
  - Architecture documentation with diagrams
  - Testing guide
  - Video demo script
  - API documentation (inline)
  - Build instructions
  - Quick start guide

- **Build System**
  - CMake build configuration
  - Support for Release/Debug builds
  - Automated test discovery
  - Install targets

- **Utilities**
  - Server startup script
  - Client startup scripts
  - Shared memory cleanup script
  - Sample commands
  - Demo data

### Documentation
- Full project README with quick start
- Architecture documentation with system diagrams
- Testing procedures and manual tests
- Video demo script with timing and narration
- Build guide with troubleshooting
- Contributing guidelines
- This changelog

## Future Roadmap

### [1.1.0] - Planned
- [ ] User list display (socket mode)
- [ ] Message history persistence
- [ ] Database backend (SQLite)
- [ ] User authentication
- [ ] Message search functionality
- [ ] Typing indicators
- [ ] Message reactions/emoji
- [ ] Group chat support

### [1.2.0] - Planned
- [ ] TLS encryption for sockets
- [ ] File sharing capability
- [ ] Video/Audio call basic support
- [ ] Message encryption
- [ ] User profiles
- [ ] Presence/away status

### [2.0.0] - Major Release
- [ ] Cluster support for socket server
- [ ] Distributed shared memory
- [ ] Mobile client (Qt for Android/iOS)
- [ ] REST API
- [ ] Web client
- [ ] Message bots
- [ ] Plugin architecture

---

## How to Upgrade

Currently at version 1.0.0 (initial release). Future upgrades will include migration guides.

---

## Version Information

**Current Version:** 1.0.0  
**Release Date:** 2025-12-08  
**Status:** Stable  
**License:** MIT
