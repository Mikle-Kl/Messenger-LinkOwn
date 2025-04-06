#include "client.h"

int main() {
        
    system("chcp 1251 > nul");

    Client client;
    if (!client.connect_to_server("127.0.0.1", 4444)) {
        std::cerr << "Unable to connect. Exiting..." << std::endl;
        return 1;
    }
    std::cout << "Введите никнейм: ";
    std::string nickname;
    std::getline(std::cin, nickname);

    std::string msg;
    
    client.start_receiving();
    client.send_message("/nick:"+nickname);
    while(true){
        std::cout << ">";
        std::getline(std::cin, msg);
     
        if (msg=="/h")
        {
            std::cout << "Выход из приложения ('exit')" << std::endl ;
            std::cout << "Вывести список пользователей ('/users')" << std::endl ;
            continue;
        } else if (msg == "exit") {break;
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
        //client.time(); Времненно закоментил
    }

    return 0;
}
