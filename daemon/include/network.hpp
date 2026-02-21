#pragma once
#include<string>

class NetworkManager{
public:
    NetworkManager();   
    ~NetworkManager();
    
    // Initializes the socket, binds it to as port, and starts listening
    bool startServer(int port);

    // Safely closes the server socket
    void stopServer();

    // Blocks and waits for an incoming connection from the Android app
    void listenForConnections();

private:
    int server_fd;
    bool is_running;
};