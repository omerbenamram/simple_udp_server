cmake_minimum_required(VERSION 3.12)
project(simple_udp_server C)

set(CMAKE_C_STANDARD 99)

add_executable(simple_udp_server server.c)
add_executable(simple_udp_client client.c)