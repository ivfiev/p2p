cmake_minimum_required(VERSION 3.28)
project(p2p_tests C)

add_executable(p2p_tests
        ../src/cb.c
        ../src/socket.c
        ../src/peer.c
        ../src/log.c
        ../src/hashtable.c
        ../src/utils.c
        tests.c
        test.h
        fff.h
        cb_test.c
        socket_test.c
        fakes.h
        fakes.c
)

target_link_libraries(p2p_tests m)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})