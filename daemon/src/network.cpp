#include "network.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <sstream>

// struct to hold pased command
struct SyncCommand {
    std::string action;
    std::string filename;
    size_t filesize;
};

SyncCommand parseCommand(const std::string &header){
    SyncCommand cmd;
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(header);

    while(std::getline(tokenStream, token, '|')){
        tokens.push_back(token);
    }
    
    if(tokens.size() == 3){
        cmd.action = tokens[0];
        cmd.filename = tokens[1];
        try {
            cmd.filesize = std::stoull(tokens[2]);  // convert string to unsigned long long
        } catch(...) {
            cmd.filesize = 0;   // fallback if not a valid number
        }
    }

    return cmd;
}

NetworkManager::NetworkManager() : server_fd(-1), is_running(false) {}

NetworkManager::~NetworkManager(){
    stopServer();
}

bool NetworkManager::startServer(int port){
    // 1. Create socket file descriptor (IPv4, TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0){
        std::cerr << "Error: socker creation failed!\n";
        return false;
    }

    // 2. Forcefully attach socket to the port (prevents "Address already in use" errors)
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        std::cerr << "Error: setsockopt failed!\n";
        return false;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;   // Listen to all available interfaces
    address.sin_port = htons(port);         // Convert port to network byte order

    // 3. Bind the socket to the network address and port
    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        std::cerr << "Error: Bind failed! Port might be in use.\n";
        return false;
    }

    // 4. Start listening for incoming connections (allow a backlog of 3 pending connections)
    if(listen(server_fd, 3) < 0){
        std::cerr << "Error: Listen failed!\n";
        return false;
    }

    is_running = true;
    std::cout << "Server listening on port " << port << std::endl;
    return true;
}


void NetworkManager::listenForConnections(){
    struct sockaddr_in address;
    int addrLen = sizeof(address);

    while(is_running){
        std::cout<<"Waiting for connection from Android app...\n";
        
        // 5. Accept an incoming connection (This blocks until a client connects)
        // NOTE: accept is blocking call, code will pause and wait until android app initiates a connection
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrLen);
        if(new_socket < 0){
            std::cerr<<"Error: Accept failed!\n";
            return;
        }
    
        std::cout<<"Client connection accepted!\n";
    
    
        char buffer[1024] = {0};
        int valread = read(new_socket, buffer, 1024);

        if(valread > 0){
            std::string receivedData(buffer, valread);

            // look for newline char that splits header from payload
            size_t newlinePos = receivedData.find('\n');

            if(newlinePos != std::string::npos){
                std::string header = receivedData.substr(0, newlinePos);
                SyncCommand cmd = parseCommand(header);

                std::cout << "[Protocol] Action: " << cmd.action << std::endl;
                std::cout << "[Protocol] File: " << cmd.filename << std::endl;
                std::cout << "[Protocol] Size: " << cmd.filesize << " bytes" << std::endl;

                if(cmd.action == "UPLOAD"){
                    std::cout << "Ready to save file to disk...\n";
                }
            } else {
                std::cout << "[Network] Raw string received: " << receivedData << std::endl;
            }
        }
    
        // Read incoming data in loop until client disconnects
        while((valread = read(new_socket, buffer, 1024)) > 0){
            std::cout << "[Network] Received: " << buffer;
            memset(buffer, 0, sizeof(buffer));   // clear buffer for next message
        }
    
        // will add the logic to actually read/write data with the Android app later.
        // For now, just close the client socket after accepting it to test connection.
        std::cout << "Client disconnected.\n";
        close(new_socket);
    }
}


void NetworkManager::stopServer(){
    if(server_fd != -1){
        close(server_fd);
        server_fd = -1;
        is_running = false;
        std::cout << "Server stopped.\n";
    }
}