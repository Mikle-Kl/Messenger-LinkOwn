#include "client.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
#endif

Client::Client() : client_socket(-1) {}

Client::~Client() {
#ifdef _WIN32
    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
        WSACleanup();
    }
#else
    if (client_socket != -1) {
        close(client_socket);
    }
#endif
}

void Client::connect_to_server(const std::string& address, int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return;
    }
#endif

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (
        client_socket == -1
#ifdef _WIN32
        || client_socket == INVALID_SOCKET
#endif
    ) {
        std::cerr << "Error creating socket!" << std::endl;
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(address.c_str());

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Connection failed!" << std::endl;
        return;
    }

    std::cout << "Connected to server!" << std::endl;
}

void Client::send_message(const std::string& message) {
    send(client_socket, message.c_str(), message.size(), 0);
}
