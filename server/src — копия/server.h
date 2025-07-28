#ifndef SERVER_H
#define SERVER_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS // Деактивирует предупреждения о старых функциях Winsock
#include <winsock2.h> // Для работы с сокетами
#include <ws2tcpip.h> // Для подключения новых функций TCP/IP
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") // Для линковки с библиотекой Winsock

#include <string>
#include <thread> // Для создания потоков
#include <unordered_map> //Ключ-значение Хэш-таблица
#include <mutex> //Синхронизации доступа к общим ресурсам между потоками
#include <unordered_set>


class Server {
public:
    Server(); 
    ~Server(); 
    void start(int port); 
    void handle_client(SOCKET client_socket); 
    bool broadcast(SOCKET from_socket, const char* data, int length);
    bool handle_command(SOCKET client_socket, const std::string& msg);
    void remove_client(SOCKET client_socket);
private:
    std::mutex clients_mutex;  
    std::unordered_map<SOCKET, std::string> users;  
    std::unordered_set<SOCKET> clients;  
    SOCKET server_socket; 
    bool wsa_initialized = false;
};

    

#endif // SERVER_H