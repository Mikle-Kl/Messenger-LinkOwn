#include "client.h"
#include <limits>

constexpr int BUFFER_SIZE = 1024;
constexpr const char* HELP_TEXT =
    "Выход из приложения ('exit')\n"
    "Вывести список пользователей ('/users')\n"
;

Client::Client() : client_socket(-1), running(false) {
    #ifdef _WIN32
        WSADATA wsaData; 
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed!" << std::endl;
            std::exit(1);
        }
    #endif
}

Client::~Client() { 
    stop(); 
    #ifdef _WIN32
        WSACleanup();
    #endif
}

// Подключение к серверу
bool Client::connect_to_server(const std::string& address, int port) {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (
        client_socket == -1
        #ifdef _WIN32
                || client_socket == INVALID_SOCKET 
        #endif
    ) {
        std::cerr << "Ошибка создания сокета!" << std::endl;
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(address.c_str());

    #ifdef _WIN32
        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    #else
        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    #endif
    {
        print_socket_error("Ошибка подключения!");
        return false;
    }

    std::cout << "Подключение успешно!" << std::endl;
    return true;
}

// Отправка сообщения
bool Client::send_message(const std::string& message) {
    const char* data = message.c_str();
    if (message.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        std::cerr << "Сообщение слишком большое для отправки!" << std::endl;
        return false;
    }
    int length = static_cast<int>(message.size()); 
    int bytes_sent = send(client_socket, data, length, 0);

    std::lock_guard<std::mutex> lock(output_mutex);
    if (bytes_sent == -1
    #ifdef _WIN32
            || bytes_sent == SOCKET_ERROR
    #endif
    ) {
        print_socket_error("Ошибка отправки!");
        return false;
    } 
    return true;
}

// Начать прослушивание в отдельном потоке
void Client::start_receiving() {
    running = true;
    receiver_thread = std::thread([this]() {
        char buffer[BUFFER_SIZE] = {0};
        int bytes_received;

        while (running) {
            memset(buffer, 0, sizeof(buffer));
            bytes_received = recv(this->client_socket, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received > 0) {
                std::string message(buffer, bytes_received);
                std::lock_guard<std::mutex> lock(output_mutex);
                std::cout << "\r\33[2K" << "[Новое сообщение]: " << message << std::endl << "> ";
                std::cout.flush();
            }
            else if (bytes_received == 0) {
                std::lock_guard<std::mutex> lock(output_mutex);
                std::cout << "[Сервер закрыл соединение]\n";
                running = false;
                break;
            }
            else { 
                std::lock_guard<std::mutex> lock(output_mutex);
                #ifdef _WIN32
                    int error_code = WSAGetLastError();
                    if (error_code == WSAEINTR) {
                        continue;
                    }
                    std::cerr << "Ошибка при получении (recv): " << error_code << std::endl;
                #else
                    perror("recv");
                #endif
                running = false;
                break;
            }
        }
    });
}

// Остановка работы клиента
void Client::stop() {
    running = false;

    #ifdef _WIN32
        if (client_socket != INVALID_SOCKET) {
            shutdown(client_socket, SD_BOTH);
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
        }
    #else
        if (client_socket != -1) {
            shutdown(client_socket, SHUT_RDWR);
            close(client_socket);
            client_socket = -1;
        }
    #endif

    if (receiver_thread.joinable()) {
        receiver_thread.join();
    }
}

// Установка никнейма на сервере
bool Client::set_nickname() {
    std::string nickname;
    while (true) {
        std::cout << "Введите никнейм: ";
        std::getline(std::cin, nickname);
        send_message("/nick:" + nickname);

        char buf[1024];
        int len = recv(get_socket(), buf, sizeof(buf) - 1, 0);
        if (len <= 0) {
            std::cerr << "Ошибка при получении ответа от сервера." << std::endl;
            return false;
        }

        buf[len] = '\0';
        std::string reply(buf);
        std::cout << reply << std::endl;

        if (reply.find("уже занят") != std::string::npos) {
            std::cout << "Этот ник уже занят, попробуйте другой." << std::endl;
        } else if (reply.find("установлен") != std::string::npos) {
            return true;
        } else {
            std::cout << "Неожиданный ответ сервера: " << reply << std::endl;
        }
    }
}

// Вывод ошибок
void Client::print_socket_error(const std::string& msg) {
    #ifdef _WIN32
        std::cerr << msg << ": " << WSAGetLastError() << std::endl;
    #else
        perror(msg.c_str());
    #endif
}

// Запуск чата
void Client::run_chat() {
    start_receiving();
    std::cout << "\n--- Чат LinkOwn --- \n# Пользовательские команды ('/h')\n";
    std::string msg;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, msg);

        if (!handle_command(msg)) break;
    }
}

// Обработка пользовательского ввода
bool Client::handle_command(const std::string& msg) {
    if (msg == "/h") {
        std::cout << HELP_TEXT;
    }
    else if (msg == "exit") {
        stop();
        return false;
    }
    else if (msg.empty()) {
        std::cout << "Сообщение не может быть пустым. Попробуйте снова." << std::endl;
    }
    else if (msg == "/users") {
        send_message(msg);
    }
    else {
        if (!send_message(msg)) {
            std::cerr << "Ошибка отправки. Завершаем работу клиента." << std::endl;
            return false;
        }
    }
    return true;
}
