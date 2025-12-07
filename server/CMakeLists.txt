# Socket Chat Server
add_executable(chat_server
    server.cpp
    client_handler.cpp
    client_handler.h
)

target_link_libraries(chat_server 
    PRIVATE 
    Threads::Threads
)

target_include_directories(chat_server 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
