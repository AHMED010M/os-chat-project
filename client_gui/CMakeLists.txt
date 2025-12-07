# Qt5 Chat Client GUI
add_executable(chat_client
    main.cpp
    MainWindow.cpp
    MainWindow.h
    SocketClient.cpp
    SocketClient.h
    ShmClient.cpp
    ShmClient.h
)

target_link_libraries(chat_client 
    PRIVATE 
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Threads::Threads
    rt  # For POSIX semaphores and shared memory
)

target_include_directories(chat_client 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Set output directory
set_target_properties(chat_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/client_gui"
)
