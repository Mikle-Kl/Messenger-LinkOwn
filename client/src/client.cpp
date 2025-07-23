#include "client.h"
#include <limits>



Client::Client() : client_socket(-1), running(false) {}

Client::~Client() { stop(); }

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

    // Проверка на ошибку подключения
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

    std::cout << "Подключение успешно!\nПользовательские команды ('/h')" << std::endl;
    return true;
}

bool Client::send_message(const std::string& message) {
    // Преобразуем строку в указатель на C-style строку
    const char* data = message.c_str();
    if (message.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        std::cerr << "Message is too large to send!" << std::endl;
        return false;
    }
    int length = static_cast<int>(message.size()); 
    int bytes_sent = send(client_socket, data, length, 0);

    // Блокируем вывод для синхронизации
    std::lock_guard<std::mutex> lock(output_mutex);
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
    return true;
}

void Client::receive_messages() {
    running = true;
    receiver_thread = std::thread([this]() {
        char buffer[1024] = {0};
        int bytes_received;
        
        while (running) {
            bytes_received = recv(this->client_socket, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received > 0) {
                std::string message(buffer, bytes_received);  // создаём строку ИМЕННО с нужной длиной

                std::lock_guard<std::mutex> lock(output_mutex);
                
                std::cout << "\r\33[2K";  // \r - возврат каретки в начало, \33[2K - очистка всей строки
                std::cout << "[Новое сообщение]: " << message << std::endl;
                std::cout << "> ";
                std::cout.flush();
            }
            else if (bytes_received == 0) { // Проверка на ошибку
                std::lock_guard<std::mutex> lock(output_mutex);
                std::cout << "\n[Сервер закрыл соединение]\n";
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

void Client::stop() {
    running = false;

    #ifdef _WIN32
        if (client_socket != INVALID_SOCKET) {
            shutdown(client_socket, SD_BOTH);
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            WSACleanup();
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




