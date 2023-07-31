cmake_minimum_required (VERSION 3.0)
project (server)
link_libraries(ws2_32)
add_executable(server Server.cpp)