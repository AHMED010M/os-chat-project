cmake_minimum_required(VERSION 3.16)
project(os_chat_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt5 moc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5
find_package(Qt5 COMPONENTS 
    Core 
    Gui 
    Widgets 
    Network 
    REQUIRED
)

# Find Threads
find_package(Threads REQUIRED)

# Add subdirectories
add_subdirectory(server)
add_subdirectory(client_gui)
add_subdirectory(tests)

message(STATUS "${echo_msg}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
