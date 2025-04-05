#include "client.h"
#include <iostream>

int main() {
    Client client;
    client.connect_to_server("127.0.0.1", 4444);

    std::string msg;
    std::cout << "Enter message to send: ";
    std::getline(std::cin, msg);

    client.send_message(msg);
    return 0;
}
