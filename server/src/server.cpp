#include "server.h"
#include <iostream>
#include <algorithm>
#include <unordered_set>


std::unordered_map<int, std::string> users;  // сокет -> никнейм
std::unordered_set<SOCKET> clients;  // для быстрого поиска
std::mutex clients_mutex;  // Для синхронизации потоков

// Конструктор сервера
Server::Server() : server_socket(-1) {}
// Деструктор сервера
Server::~Server() {
    #ifdef _WIN32
        if (server_socket != INVALID_SOCKET) {
            closesocket(server_socket);
            WSACleanup();
        }
    #else
        if (server_socket != -1) {
            close(server_socket);
        }
    #endif
}

// Метод для обработки клиента
void Server::handle_client(SOCKET client_socket) {
    char buffer[1024];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';  // Завершаем строку
        std::string msg(buffer);
        

        std::lock_guard<std::mutex> lock(clients_mutex);

        std::cout << "Сообщение от клиента с сокетом - " << client_socket << ": " << msg << std::endl;

        // Установка никнейма
        if (msg.substr(0, 6) == "/nick:") {
            std::string nickname = msg.substr(6);
            users[client_socket] = nickname;
            std::cout << "Никнейм клиента с сокетом - " << client_socket << " записан как: " << nickname << std::endl;
            continue;
        }

        // Команда /users
        if (msg.substr(0, 6) == "/users") {
            std::string user_list = "(SERVER) Активные пользователи:\n";
            for (const auto& [sock, name] : users) {
                user_list += "- " + name + "\n";
            }
            
            std::cout << "Отправлен список пользоватей клиенту с сокетом " << client_socket << std::endl;
            send(client_socket, user_list.c_str(), user_list.size(), 0);
            continue;
        }

        // Пересылка сообщений другим клиентам
        for (SOCKET client : clients) {
            if (client != client_socket) {
                send(client, buffer, bytes_received, 0);
            }
        }
    }
    
    // Удаляем клиента, если соединение закрыто
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::cout << "Клиент с сокетом " << client_socket << " отключился." << std::endl;
    clients.erase(client_socket);
    if (users.count(client_socket)) {
        std::cout << "Удаление никнейма: " << users[client_socket] << std::endl;
        users.erase(client_socket);
    }

    #ifdef _WIN32
        closesocket(client_socket);
    #else
        close(client_socket);
    #endif
}

// Метод для запуска сервера
void Server::start(int port) {
    #ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return;
        }
    #endif
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        return;
    }

    if (listen(server_socket, 10) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        return;
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed!" << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.insert(client_socket);
        }

        std::thread(&Server::handle_client, this, client_socket).detach();
    }
}
