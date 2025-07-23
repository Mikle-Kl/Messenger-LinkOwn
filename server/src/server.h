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
    Server(); // Конструктор
    ~Server(); // Деструктор
    void start(int port); // Метод для запуска сервера
    void handle_client(SOCKET client_socket); // Метод для обработки клиента
    void broadcast(SOCKET from_socket, const char* data, int length);
    bool handle_command(SOCKET client_socket, const std::string& msg);

private:
    std::mutex clients_mutex;  // Для синхронизации потоков
    std::unordered_map<SOCKET, std::string> users;  // сокет -> никнейм
    std::unordered_set<SOCKET> clients;  // для быстрого поиска
    SOCKET server_socket; // Сокет сервера
    bool wsa_initialized = false;
};

    

#endif // SERVER_H