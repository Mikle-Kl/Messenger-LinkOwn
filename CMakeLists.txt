cmake_minimum_required(VERSION 3.10)
project(Linkown)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(server src/server/server.cpp)