#include "client.h"

int main() {
        
    #ifdef _WIN32
        system("chcp 65001 > nul");
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

    std::cout << "Введите никнейм: ";
    std::string nickname;
    std::getline(std::cin, nickname);

    std::string msg;
    
    client.receive_messages();
    client.send_message("/nick:"+nickname);
    
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
        if (!client.send_message(nickname+":"+msg)) {
            std::cerr << "Ошибка отправки. Завершаем работу клиента." << std::endl;
            break;
        }
    }


    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}
