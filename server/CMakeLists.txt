cmake_minimum_required(VERSION 3.10)
project(Linkown)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(server src/server.cpp src/server.h src/s_main.cpp)

# Подключаем библиотеку для Windows (Winsock)
if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()