//{cmake --build .} в папке build
#include "server.h"

int main() {
    Server server;
    server.start(4444); // Запуск сервера на порту 8080
    return 0;
}
