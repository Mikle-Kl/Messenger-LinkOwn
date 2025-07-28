#include "server.h"
#include <iostream>


Server::Server() : server_socket(INVALID_SOCKET) {}

Server::~Server() {
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
    if (wsa_initialized) {
        WSACleanup();
    }
}
//Обработка сообщений
void Server::handle_client(SOCKET client_socket) {
    char buffer[1024];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        std::string msg(buffer, bytes_received);

        std::cout << "Сообщение от клиента с сокетом - " << client_socket << ": " << msg << std::endl;

        if (handle_command(client_socket, msg)) { continue; } 

        std::string sender;
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = users.find(client_socket);
            sender = (it != users.end()) ? it->second : "Unknown";
        }
        std::string formatted_msg = "(" + sender + "): " + msg;
        if(!broadcast(client_socket, formatted_msg.c_str(), formatted_msg.size())) { break; }
        
    }

    remove_client(client_socket);
}

//Удаление клиента
void Server::remove_client(SOCKET client_socket)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    closesocket(client_socket);
    std::cout << "Клиент с сокетом " << client_socket << " отключился." << std::endl;
    clients.erase(client_socket);
    if (users.count(client_socket)) {
        std::cout << "Удаление никнейма: " << users[client_socket] << std::endl;
        users.erase(client_socket);
    }
}

//Обработака специальных сообщений
bool Server::handle_command(SOCKET client_socket, const std::string& msg) {
    if (msg.rfind("/nick:", 0) == 0) {
        std::string nickname = msg.substr(6);
        // Удалим пробелы в начале и конце ника
        nickname.erase(0, nickname.find_first_not_of(" \t\r\n"));
        nickname.erase(nickname.find_last_not_of(" \t\r\n") + 1);
        
        
        if (nickname.empty()) {
            std::string error = "(SERVER) Ник не может быть пустым.\n";
            send(client_socket, error.c_str(), error.size(), 0);
            return true;
        }
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            // Проверка на уникальность ника
            for (const auto& [sock, name] : users) {
                if (name == nickname) {
                    std::string error = "(SERVER) Ник '" + nickname + "' уже занят.\n";
                    send(client_socket, error.c_str(), error.size(), 0);
                    return true;
                }
            }
        
            users[client_socket] = nickname;
        }
        std::string ok_msg = "(SERVER) Ник '" + nickname + "' установлен.\n"; 
        send(client_socket, ok_msg.c_str(), ok_msg.size(), 0);                
        std::cout << "Никнейм клиента с сокетом - " << client_socket << " записан как: " << nickname << std::endl;
        return true;
    }

    if (msg == "/users") {
        std::string user_list = "(SERVER) Активные пользователи:\n";
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (const auto& [sock, name] : users) {
                user_list += "- " + name + "\n";
            }
        }
        std::cout << "Отправлен список пользователей клиенту с сокетом " << client_socket << std::endl;
        send(client_socket, user_list.c_str(), user_list.size(), 0);
        return true;
    }

    return false; 
}

//Отправляем сообщения всем клиентам, кроме того, кто его отправил
bool Server::broadcast(SOCKET from_socket, const char* data, int length) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : clients) {
        if (client != from_socket) {
            if (send(client, data, length, 0) == SOCKET_ERROR) {
                std::cerr << "Ошибка отправки клиенту: " << client << std::endl;
                return false;
            }
        }
    }
    return true;
}

//Запустить сервер, принимать подключения и обрабатывать клиентов параллельно.
void Server::start(int port) {

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return;
    }

    wsa_initialized = true;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if ( bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR ) {
        std::cerr << "Ошибка привязки сокета" << std::endl;
        return;
    }

    if ( listen(server_socket, 10) == SOCKET_ERROR ) {
        std::cerr << "Ошибка прослушивания порта" << std::endl;
        return;
    }

    std::cout << "Сервер слушает на порту " << port << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Ошибка подключения клиента!" << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.insert(client_socket);
        }

        std::thread(&Server::handle_client, this, client_socket).detach();
    }
}
