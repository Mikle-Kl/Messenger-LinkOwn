//{cmake --build .} в папке build
#include "server.h"

int main() {
    
    system("chcp 65001 > nul");

    Server server;
    server.start(4444); // Запуск сервера на порту 8080
    return 0;
}
