#ifndef SERVER_H
#define SERVER_H
#include <string>
#include <sstream>
#include <thread> // Для создания потоков
#include <unordered_map> //Ключ-значение Хэш-таблица
#include <mutex> //Синхронизации доступа к общим ресурсам между потоками
#include <vector>

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS // Деактивирует предупреждения о старых функциях Winsock
    #include <winsock2.h> // Для работы с сокетами
    #include <ws2tcpip.h> // Для подключения новых функций TCP/IP
    #pragma comment(lib, "ws2_32.lib") // Для линковки с библиотекой Winsock
#else
    #include <sys/types.h> // Включает определения базовых типов данных, которые используются в системных вызовах
    #include <sys/socket.h> // Включает основные определения для работы с сокетами
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> // Для close(), если на Linux
    #include <errno.h>
#endif

class Server {
public:
    Server(); // Конструктор
    ~Server(); // Деструктор
    void start(int port); // Метод для запуска сервера
    void handle_client(int client_socket); // Метод для обработки клиента
    void store_data(const std::string& key, const std::string& value); // Метод для хранения данных
private:
    std::unordered_map<std::string, std::string> store; // Простое хранилище данных (ключ-значение)
    std::mutex store_mutex; // Мьютекс для синхронизации доступа к хранилищу данных
    int server_socket; // Сокет сервера
};

    

#endif // SERVER_H