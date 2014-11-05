cmake_minimum_required(VERSION 2.8.4)
project(cmput379Assignment2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(SOURCE_FILES server_f.c)
add_executable(server_f ${SOURCE_FILES})