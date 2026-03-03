#include "discovery.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>

DiscoveryManager::DiscoveryManager() : udp_socket(-1), is_running(false) {}

DiscoveryManager::~DiscoveryManager(){
    stopBroadcasting();
}

void DiscoveryManager::startBroadcasting(int tcp_port){
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket < 0){
        std::cerr << "[Discovery] Error: Failed to create UDP socket.\n";
        return;
    }

    // enable broadcast permission on this socket
    int broadcast_enable=1;
    if(setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0){
        std::cerr << "[Discovery] Error: Failed to enable broadcast.\n";
        return;
    }

    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(8888);  // android will listen on port 8888
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");  // universal broadcast IP

    is_running = true;
    std::string message = "LITSYNC_SERVER|" + std::to_string(tcp_port);

    std::cout << "[Discovery] Broadcasting presence on UDP port 8888...\n";

    while(is_running){
        sendto(udp_socket, message.c_str(), message.length(), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));

        // sleep for 2 seconds before shouting again
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void DiscoveryManager::stopBroadcasting(){
    is_running = false;
    if(udp_socket != -1){
        close(udp_socket);
        udp_socket = -1;
        std::cout << "[Discovery] Stopped broadcasting.\n";
    }
}