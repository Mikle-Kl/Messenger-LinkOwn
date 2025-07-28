#include "server.h"
#include <iostream>

constexpr int BUFFER_SIZE = 1024;
constexpr const char* CMD_NICK = "/nick:";
constexpr const char* CMD_SHOW_USERS = "/users";


Server::Server() : server_socket(INVALID_SOCKET) {}
Server::~Server() {
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
    if (wsa_initialized) {
        WSACleanup();
    }
}

//Запустить сервер, принимать подключения и обрабатывать клиентов параллельно.
void Server::start(int port) {
    if (!initialize_winsock()) return;
    if (!create_socket()) return;
    if (!bind_socket(port)) return;
    if (!listen_socket()) return;

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

// Иницилизация Winsock
bool Server::initialize_winsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    wsa_initialized = true;
    return true;
}

// Создание сокета IPv4 TCP Listen
bool Server::create_socket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return false;
    }
    return true;
}

bool Server::bind_socket(int port) {
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка привязки сокета" << std::endl;
        return false;
    }
    return true;
}

bool Server::listen_socket() {
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        std::cerr << "Ошибка прослушивания порта" << std::endl;
        return false;
    }
    return true;
}


// Обработка сообщений
void Server::handle_client(SOCKET client_socket) {
    while (true) {
        std::string msg = receive_message(client_socket);
        if (msg.empty()) break;

        log_client_message(client_socket, msg);

        if (handle_command(client_socket, msg)) continue;

        std::string formatted_msg = format_message(client_socket, msg);
        if (!broadcast(client_socket, formatted_msg)) break;
    }

    remove_client(client_socket);
}

// Отправка сообщений
std::string Server::receive_message(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) return "";

    buffer[bytes_received] = '\0';
    return std::string(buffer);
}

// Шаблон для логов
void Server::log_client_message(SOCKET client_socket, const std::string& msg) {
    std::cout << "Сообщение от клиента с сокетом - " << client_socket << ": " << msg << std::endl;
}

// Формат получения сообщений заданный сервером 
std::string Server::format_message(SOCKET client_socket, const std::string& msg) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = users.find(client_socket);
    std::string sender = (it != users.end()) ? it->second : "Unknown";
    return "(" + sender + "): " + msg;
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
    if (msg.rfind(CMD_NICK, 0) == 0) {
        std::string nickname = parse_nickname(msg);
        return process_nickname(client_socket, nickname);
    } else if (msg == CMD_SHOW_USERS) {
        send_user_list(client_socket);
        return true;
    }
    return false;
}

// Удаление пробелов 
std::string Server::parse_nickname(const std::string& msg) {
    constexpr size_t prefix_len = strlen(CMD_NICK); // лучше так, чем хардкодить 6
    std::string nickname = msg.substr(prefix_len);
    nickname.erase(0, nickname.find_first_not_of(" \t\r\n"));
    nickname.erase(nickname.find_last_not_of(" \t\r\n") + 1);
    return nickname;
}

// Создания никнейма пользователя
bool Server::process_nickname(SOCKET client_socket, const std::string& nickname) {
    if (nickname.empty()) {
        send_message(client_socket, "(SERVER) Ник не может быть пустым.\n");
        return true;
    }

    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& [sock, name] : users) {
        if (name == nickname) {
            send_message(client_socket, "(SERVER) Ник '" + nickname + "' уже занят.\n");
            return true;
        }
    }

    users[client_socket] = nickname;
    send_message(client_socket, "(SERVER) Ник '" + nickname + "' установлен.\n");
    std::cout << "Никнейм клиента с сокетом - " << client_socket << " записан как: " << nickname << std::endl;
    return true;
}

// Отправка списка пользователя
void Server::send_user_list(SOCKET client_socket) {
    std::string list = "(SERVER) Активные пользователи:\n";
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto& [sock, name] : users) {
            list += "- " + name + "\n";
        }
    }
    send_message(client_socket, list);
    std::cout << "Отправлен список пользователей клиенту с сокетом " << client_socket << std::endl;
}

// Отправка сообщений
bool Server::send_message(SOCKET client_socket, const std::string& msg) {
    return send(client_socket, msg.c_str(), msg.size(), 0) != SOCKET_ERROR;
}



//Отправляем сообщения всем клиентам, кроме того, кто его отправил
bool Server::broadcast(SOCKET from_socket, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    bool all_ok = true;
    for (SOCKET client : clients) {
        if (client != from_socket) {
            if (!send_message(client, message)) {
                std::cerr << "Ошибка отправки клиенту: " << client << std::endl;
                all_ok = false;
            }
        }
    }

    return all_ok;
}

