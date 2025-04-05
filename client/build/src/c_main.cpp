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
    std::cout << "Введите никнейм: ";
    std::string nickname;
    std::getline(std::cin, nickname);

    std::string msg;
    std::cout << "Если вы хотите выйти ('exit')" << std::endl ;
    client.start_receiving();
    while(true){
        std::cout << ">";
        std::getline(std::cin, msg);

        if (msg == "exit") break;
        if (msg.empty()) {
            std::cout << "Сообщение не может быть пустым. Попробуйте снова." << std::endl;
            continue;
        }
        
        if (!client.send_message(nickname+":"+msg)) {
            std::cerr << "Ошибка отправки. Завершаем работу клиента." << std::endl;
            break;
        }
        //client.time(); Времненно закоментил
    }

    return 0;
}
