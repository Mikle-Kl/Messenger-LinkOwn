#ifndef CLIENT_H
#define CLIENT_H

#include <string>

class Client {
public:
    Client();
    ~Client();
    void connect_to_server(const std::string& address, int port);
    void send_message(const std::string& message);

private:
    int client_socket;
};

#endif // CLIENT_H
