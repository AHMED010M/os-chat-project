# Contributing to OS Chat Project

We welcome contributions! This project is designed as an educational platform for learning OS concepts.

## How to Contribute

### Reporting Issues

1. Check if issue already exists
2. Provide:
   - OS and version (e.g., Ubuntu 20.04)
   - Build output/error messages
   - Steps to reproduce
   - Expected vs actual behavior

### Submitting Changes

1. **Fork** the repository
2. **Create branch**: `git checkout -b feature/your-feature`
3. **Make changes**: Follow code style (see below)
4. **Test**: Run `ctest` and verify no regressions
5. **Commit**: `git commit -m "Clear description of changes"`
6. **Push**: `git push origin feature/your-feature`
7. **Pull Request**: Create PR with description

### Code Style

**C++:**
- Use C++17 features
- Follow Google C++ Style Guide
- 4-space indentation
- Meaningful variable names
- Comments for non-obvious logic

**Example:**
```cpp
// Process client connection
void handle_client(int socket_fd) {
    Message msg;
    while (recv_message(socket_fd, msg)) {
        // Update timestamp
        std::string ts = Message::get_current_timestamp();
        strncpy(msg.timestamp, ts.c_str(), MAX_TIMESTAMP_LEN - 1);
        
        // Broadcast to others
        broadcast_message(msg);
    }
}
```

### Testing

Before submitting:

```bash
# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Test
ctest --verbose

# Check for memory leaks
valgrind --leak-check=full ./tests/test_socket
valgrind --leak-check=full ./tests/test_shm
```

## Areas for Contribution

### Bug Fixes
- Concurrency issues
- Memory leaks
- UI glitches
- Build problems

### Features
- User list in SHM mode
- Message persistence (database)
- File sharing
- Encryption (TLS for sockets)
- Authentication
- Group chatrooms
- Message search
- Typing indicators

### Performance
- Thread pool for server
- Lock-free data structures
- epoll/kqueue for I/O
- SHM buffer optimization

### Documentation
- API documentation
- Tutorial improvements
- Example code
- Video subtitles

### Testing
- Additional test cases
- Integration tests
- Performance benchmarks
- Stress tests

## Code Review Checklist

PRs are reviewed for:
- ✅ C++17 compliance
- ✅ Thread safety
- ✅ Memory leaks
- ✅ Error handling
- ✅ Documentation
- ✅ Test coverage
- ✅ Code style
- ✅ No regressions

## Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Type:** feat, fix, docs, style, refactor, test, chore

**Example:**
```
feat(server): add thread pool for client handling

Implement a configurable thread pool instead of creating a new thread
for each client. This improves server scalability for hundreds of
concurrent clients.

Fixes: #42
```

## Getting Help

- Check documentation in `docs/`
- Read existing code comments
- Open discussion issue
- Ask in PR review comments

## License

By contributing, you agree your code will be MIT Licensed.

---

Thank you for contributing to OS Chat Project! 🙋
