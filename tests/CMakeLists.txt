# Test Suite
include(CTest)

# Socket System Tests
add_executable(test_socket test_socket.cpp)
target_link_libraries(test_socket PRIVATE Threads::Threads)
target_include_directories(test_socket PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
add_test(NAME SocketTests COMMAND test_socket)

# Shared Memory Tests
add_executable(test_shm test_shm.cpp)
target_link_libraries(test_shm PRIVATE Threads::Threads rt)
target_include_directories(test_shm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
add_test(NAME ShmTests COMMAND test_shm)

message(STATUS "Tests configured. Run 'ctest' or 'make test' to execute.")
