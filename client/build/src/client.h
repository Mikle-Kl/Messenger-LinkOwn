#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // В зависимости от компилятора пользователя добовляем
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
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>
#include <mutex>

class Client {
public:
    Client();
    ~Client();
    SOCKET get_socket() const;
    bool connect_to_server(const std::string& address, int port);
    bool send_message(const std::string& message);
    void start_receiving();
    void receive_messages();
    void time();
private:
    int client_socket;
};


#endif // CLIENT_H
