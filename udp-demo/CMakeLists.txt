cmake_minimum_required(VERSION 3.6)
project(udp_demo)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(SRC_CLI client.c)
set(SRC_CLING client-ng.c)
set(SRC_SRV server.c)
set(SRC_SRVNG server-ng.c)
add_executable(client ${SRC_CLI})
add_executable(client-ng ${SRC_CLING})
add_executable(server ${SRC_SRV})
add_executable(server-ng ${SRC_SRVNG})
