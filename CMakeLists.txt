cmake_minimum_required(VERSION 2.8.4)
project(cmput379Assignment2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(BASE_SOURCES server.c concurrent_handler.h request_handler.h request_handler.c)
add_executable(server_f forked_handler.c ${BASE_SOURCES})
add_executable(server_p threaded_handler.c ${BASE_SOURCES})

target_link_libraries(server_p pthread)