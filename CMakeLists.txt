cmake_minimum_required(VERSION 3.28)
project(p2p C)

set(CMAKE_C_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(p2p
        src/main.c
        src/cb.c
        src/socket.c
        src/peer.c
        src/log.c
        src/hashtable.c
        src/utils.c
        src/app.c
)

target_link_libraries(p2p m)

add_subdirectory(test)