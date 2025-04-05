#include "server.h"
#include <iostream>

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

// Метод для хранения данных
void Server::store_data(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(store_mutex); // Защищаем доступ к store
    store[key] = value;
}

// Метод для обработки клиента
void Server::handle_client(int client_socket) {
    // Пример обработки клиента: чтение данных и сохранение в хранилище
    char buffer[1024];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0'; // Завершаем строку
        std::string client_data(buffer);
        std::cout << "Received from client: " << client_data << std::endl;
        store_data("some_key", client_data); // Пример сохранения данных
    }
    #ifdef _WIN32
            closesocket(client_socket);
    #else
            close(client_socket);
    #endif // Закрываем сокет клиента
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
    // Создание сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        return;
    }

    // Прослушивание порта
    if (listen(server_socket, 10) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        return;
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        // Принятие подключения
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket != -1) {
            std::thread(&Server::handle_client, this, client_socket).detach();
        }
    }
}
