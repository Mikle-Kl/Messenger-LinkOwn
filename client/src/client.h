#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // В зависимости от компилятора 
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
#endif
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex> //подключает механизмы синхронизации потоков


class Client {
public:
    Client();
    ~Client();
    bool connect_to_server(const std::string& address, int port);
    bool send_message(const std::string& message);
    void receive_messages();
    void stop();
private:
    #ifdef _WIN32
        SOCKET client_socket;
    #else
        int client_socket;
    #endif
    std::atomic<bool> running;
    std::thread receiver_thread;
    std::mutex output_mutex;
};


#endif // CLIENT_H
