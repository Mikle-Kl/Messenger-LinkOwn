cmake_minimum_required(VERSION 3.10)
project(Linkown)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(client src/client.cpp src/client.h src/c_main.cpp)

# Подключаем библиотеку для Windows (Winsock)
if(WIN32)
    target_link_libraries(client PRIVATE ws2_32)
endif()