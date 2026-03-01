#include "network.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>  // for file writing

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
    // create socket file descriptor (IPv4, TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0){
        std::cerr << "[Network] Error: socker creation failed!\n";
        return false;
    }

    // forcefully attach socket to the port (prevent "Address already in use" errors)
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        std::cerr << "[Network] Error: setsockopt failed!\n";
        return false;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;   // listen to all available interfaces
    address.sin_port = htons(port);         // convert port to network byte order

    // bind the socket to the network address and port
    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        std::cerr << "[Network] Error: Bind failed! Port might be in use.\n";
        return false;
    }

    // 4. Start listening for incoming connections (allow a backlog of 3 pending connections)
    if(listen(server_fd, 3) < 0){
        std::cerr << "[Network] Error: Listen failed!\n";
        return false;
    }

    is_running = true;
    std::cout << "[Network] Server listening on port " << port << std::endl;
    return true;
}


void NetworkManager::listenForConnections(SyncManager& syncManager){
    struct sockaddr_in address;
    int addrLen = sizeof(address);

    while(is_running){
        std::cout<<"[Network] Waiting for connection from Android app...\n";
        
        // accept an incoming connection (This blocks until a client connects)
        // NOTE: accept is blocking call, code will pause and wait until android app initiates a connection
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrLen);
        if(new_socket < 0){
            std::cerr<<"[Network] Error: Accept failed!\n";
            return;
        }
    
        std::cout<<"[Network] Client connection accepted!\n";
    
    
        char buffer[4096] = {0};
        int valread = read(new_socket, buffer, sizeof(buffer));

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
                    // std::cout << "Muting inotify for: " << cmd.filename << std::endl;
                    syncManager.ignoreFile(cmd.filename);     // lock and add file to ignore list

                    // path where the file will be saved
                    std::string filepath = "./litsync_folder/" + cmd.filename;
                    std::ofstream outfile(filepath, std::ios::binary);

                    if(outfile.is_open()){
                        size_t total_received = 0;

                        // calculate if any file data came attached to the first header packet
                        size_t payload_start = newlinePos + 1;
                        size_t initial_data_length = valread - payload_start;

                        if(initial_data_length > 0){
                            outfile.write(buffer + payload_start, initial_data_length);
                            total_received += initial_data_length;
                        }

                        // loop and download the rest of the file chunks
                        while(total_received < cmd.filesize){
                            int bytes_read = read(new_socket, buffer, sizeof(buffer));
                            if(bytes_read < 0) break;   // network error or client disconnected early

                            outfile.write(buffer, bytes_read);
                            total_received += bytes_read;
                        }

                        outfile.close();
                        std::cout << "[Network] Successfully saved " << cmd.filename << " to disk!\n";
                    } else {
                        std::cerr << "[Netowork] Error: Could not open file for writing: " << filepath << std::endl;
                    }
                    syncManager.unignoreFile(cmd.filename);   // unlock, remove file from ignored list
                }
            }
        }
    
        // will add the logic to actually read/write data with the Android app later.
        // For now, just close the client socket after accepting it to test connection.
        std::cout << "[Network] Client disconnected.\n";
        close(new_socket);
    }
}


void NetworkManager::stopServer(){
    if(server_fd != -1){
        close(server_fd);
        server_fd = -1;
        is_running = false;
        std::cout << "[Network] Server stopped.\n";
    }
}