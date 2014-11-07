cmake_minimum_required(VERSION 2.8.4)
project(cmput379Assignment2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(BASE_SOURCES server.h server.c concurrent_handler.h threaded_handler.c request_handler.h request_handler.c)
add_executable(server_f server.c forked_handler.c ${BASE_SOURCES})
add_executable(server_p server.c threaded_handler.c ${BASE_SOURCES})

target_link_libraries(server_p pthread)