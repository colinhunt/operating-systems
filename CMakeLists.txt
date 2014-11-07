cmake_minimum_required(VERSION 2.8.4)
project(cmput379Assignment2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(SOURCE_FILES server.h server.c concurrent_handler.h forked_handler.c request_handler.h request_handler.c)
add_executable(server_f server_f.c ${SOURCE_FILES})