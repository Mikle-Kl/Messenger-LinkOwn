#include "client.h"

int main() {
        
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);    // Устанавливаем кодировку UTF-8 для вывода
    SetConsoleCP(CP_UTF8);          // И для ввода
    #endif

    Client client;
    if (!client.connect_to_server("127.0.0.1", 4444)) {
        std::cerr << "Unable to connect. Exiting..." << std::endl;
        return 1;
    }

    std::string msg;
    while(true){
        
        std::cout << "Введите сообщение (или 'exit'): ";
        std::getline(std::cin, msg);

        if (msg == "exit") break;
        
        
        if (!client.send_message(msg)) {
            std::cerr << "Ошибка отправки. Завершаем работу клиента." << std::endl;
            break;
        }
        client.time();
        std::cout << "-------------------------------------------------------" << std::endl;
    }

    return 0;
}
