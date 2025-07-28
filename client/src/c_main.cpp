#include "client.h"

int main() {
        
    #ifdef _WIN32
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
    #endif

    // Подключение к серверу
    Client client;
    if (!client.connect_to_server("127.0.0.1", 4444)) {
        std::cerr << "Не удалось подключиться. Программа завершает работу..." << std::endl;
        return 1;
    }
    
    if (!client.set_nickname()) {
        std::cerr << "Не удалось установить никнейм. Завершение..." << std::endl;
        return 1;
    }
    
    client.run_chat();

    return 0;
}
