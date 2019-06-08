CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
project(os_net)

set(CMAKE_CXX_STANDARD 14)

add_executable(echo_server
        file_descriptor/file_descriptor.h file_descriptor/file_descriptor.cpp
        fifo/fifo.h fifo/fifo.cpp
        server/server.h server/server.cpp)
add_executable(echo_client
        file_descriptor/file_descriptor.h file_descriptor/file_descriptor.cpp
        fifo/fifo.h fifo/fifo.cpp
        client/client.h client/client.cpp)