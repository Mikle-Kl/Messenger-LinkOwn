//{ cmake --build . } в папке build
#include "server.h"

int main() {    
    SetConsoleOutputCP(CP_UTF8);

    Server server;
    server.start(4444); // Запуск сервера на порту 4444
    return 0;
}
