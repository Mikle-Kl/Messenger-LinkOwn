#include "client.h"

int main() {
        
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        WSADATA wsaData; 
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed!" << std::endl;
            return 1;
        }
    #endif

    // Подключение к серверу
    Client client;
    if (!client.connect_to_server("127.0.0.1", 4444)) {
        std::cerr << "Не удалось подключиться. Программа завершает работу..." << std::endl;
        return 1;
    }

    
    std::string nickname;
    while (true) {
        std::cout << "Введите никнейм: ";
        std::getline(std::cin, nickname);

        client.send_message("/nick:" + nickname);

        // Временно читаем ответ сами
        char buf[1024];
        int len = recv(client.get_socket(), buf, sizeof(buf) - 1, 0);
        if (len <= 0) {
            std::cerr << "Ошибка при получении ответа от сервера." << std::endl;
            return 1;
        }

        buf[len] = '\0';
        std::string reply(buf);

        std::cout << reply;

        if (reply.find("уже занят") != std::string::npos) {
            std::cout << "Этот ник уже занят, попробуйте другой." << std::endl;
            continue;
        } else if (reply.find("установлен") != std::string::npos) {
            break;
        } else {
            std::cout << "Неожиданный ответ сервера: " << reply << std::endl;
            continue;
        }
    }


    client.receive_messages();
    std::string msg;
    
    while(true){
        std::cout << "> ";
        std::getline(std::cin, msg);
     
        if (msg=="/h")
        {
            std::cout << "Выход из приложения ('exit')" << std::endl ;
            std::cout << "Вывести список пользователей ('/users')" << std::endl ;
            continue;
        } else if (msg == "exit") { 
            client.stop(); 
            break;
        } else if (msg.empty()) {
            std::cout << "Сообщение не может быть пустым. Попробуйте снова." << std::endl;
            continue;
        }else if (msg=="/users") {
            client.send_message(msg);
            continue;
        }
        if (!client.send_message(msg)) {
            std::cerr << "Ошибка отправки. Завершаем работу клиента." << std::endl;
            break;
        }
    }


    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}
