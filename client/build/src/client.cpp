#include "client.h"
#include <cstring>
#include <limits>




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

bool Client::connect_to_server(const std::string& address, int port) {
#ifdef _WIN32
    WSADATA wsaData; /////
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return false;
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
        return false;
    }

    sockaddr_in server_addr{};//////
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(address.c_str());

    // ⛔ Проверка на ошибку подключения
    if (
        #ifdef _WIN32
                connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR
        #else
                connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1
        #endif
            ) {
                std::cerr << "Connection failed!" << std::endl;
        #ifdef _WIN32
                std::cerr << "WSA Error code: " << WSAGetLastError() << std::endl;
        #else
                perror("connect"); // Выведет подробности на Linux/macOS
        #endif
            return false;
        }

    std::cout << "Подключение успешно!" << std::endl;
    return true;
}

bool Client::send_message(const std::string& message) {
    // Преобразуем строку в указатель на C-style строку
    const char* data = message.c_str();
    if (message.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        std::cerr << "Message is too large to send!" << std::endl;
        return false;
    }
    int length = static_cast<int>(message.size()); // Размер отправляемых данных
    // Безопаное преоразование size_t(беззнакового) в int с проверкой
    // Отправка сообщения
    int bytes_sent = send(client_socket, data, length, 0);

    // Проверка на ошибку
    if (bytes_sent == -1
    #ifdef _WIN32
        || bytes_sent == SOCKET_ERROR
    #endif
    ) {
        std::cerr << "Send failed!" << std::endl;
        #ifdef _WIN32
            std::cerr << "WSA Error: " << WSAGetLastError() << std::endl;
        #else
            std::cerr << "Errno: " << strerror(errno) << std::endl;
        #endif
        return false;
    } 
    std::cout << "Сообщение отправлено -- ";
    return true;
}

void Client::time(){
    // Получаем текущее системное время
    auto now = std::chrono::system_clock::now();
    // Преобразуем в time_t для использования с ctime
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    // Преобразуем time_t в строку
    std::tm* tm_ptr = std::localtime(&now_time);
    // Выводим время в стандартном формате (например, Sat Feb 25 15:30:25 2023)
    std::cout << std::put_time(tm_ptr, "%c") << std::endl;
}