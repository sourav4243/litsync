#include <iostream>
#include <filesystem>
#include <thread>
#include "network.hpp"
#include "watcher.hpp"
#include "sync_manager.hpp"
#include "discovery.hpp"

// This function runs in background thread
void runNetworkServer(NetworkManager& networkManager, SyncManager& syncManager, std::string path_to_watch){
    int port = 8080;
    if(networkManager.startServer(port)){
        // this blocks, waiting for android to connect. Won't block inotify
        networkManager.listenForConnections(syncManager, path_to_watch);
    }else{
        std::cerr << "Failed to start network server.\n";
    }
}

void runDiscoveryService(DiscoveryManager& discoveryManager, int tcp_port){
    discoveryManager.startBroadcasting(tcp_port);
}

int main(){
    std::cout << "Starting LitSync Daemon...\n";

    std::string path_to_watch;
    const char* home_dir = std::getenv("HOME");

    if(home_dir != nullptr){
        path_to_watch = std::string(home_dir) + "/LitSync";
    }else{
        std::cerr << "[Warning] Could not find HOME directory! Defaulting to local build for target folder.\n";
        path_to_watch = "./litsync_folder";
    }

    // set up the directory we want LitSync to watch
    if(!std::filesystem::exists(path_to_watch)){
        std::filesystem::create_directory(path_to_watch);
    }
    
    SyncManager syncManager;    // create state and pass to thread
    NetworkManager networkManager;
    Watcher watcher;
    DiscoveryManager discoveryManager;

    int tcp_port = 8080;

    // Launch network server in separate background thread. std::ref pass networkManager by reference
    std::thread networkThread(runNetworkServer, std::ref(networkManager), std::ref(syncManager), path_to_watch);

    // Start the UDP broadcast beacon on a background thread
    std::thread discoveryThread(runDiscoveryService, std::ref(discoveryManager), tcp_port);

    // Start filesystem watcher on main thread (this blocks forever);
    watcher.start(path_to_watch, syncManager, networkManager, 8081);

    // cleanup (if watcher ever exists)
    discoveryManager.stopBroadcasting();
    networkManager.stopServer();

    if(networkThread.joinable()){
        networkThread.join();
    }
    if(discoveryThread.joinable()){
        discoveryThread.join();
    }
    return 0;
}