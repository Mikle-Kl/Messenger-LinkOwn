#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
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
#include <thread>
#include <atomic>
#include <mutex> //подключает механизмы синхронизации потоков



class Client {
public:
    Client();
    ~Client();
    bool connect_to_server(const std::string& address, int port);
    void run_chat();
    bool set_nickname();
    bool send_message(const std::string& message);
    bool handle_command(const std::string& msg);
    void start_receiving();
    void stop();

    void print_socket_error(const std::string& msg);
    SOCKET get_socket() const { return client_socket; }

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

