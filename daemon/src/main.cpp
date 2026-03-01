#include <iostream>
#include <filesystem>
#include <thread>
#include "network.hpp"
#include "watcher.hpp"
#include "sync_manager.hpp"

// This function runs in background thread
void runNetworkServer(NetworkManager& networkManager, SyncManager& syncManager){
    int port = 8080;
    if(networkManager.startServer(port)){
        // this blocks, waiting for android to connect. Won't block inotify
        networkManager.listenForConnections(syncManager);
    }else{
        std::cerr << "Failed to start network server.\n";
    }
}

int main(){
    std::cout << "Starting LitSync Daemon...\n";

    // set up the directory we want LitSync to watch
    std::string path_to_watch = "./litsync_folder";
    if(!std::filesystem::exists(path_to_watch)){
        std::filesystem::create_directory(path_to_watch);
    }

    
    SyncManager syncManager;    // create state and pass to thread
    NetworkManager networkManager;
    Watcher watcher;

    // Launch network server in separate background thread. std::ref pass networkManager by reference
    std::thread networkThread(runNetworkServer, std::ref(networkManager), std::ref(syncManager));

    // Start filesystem watcher on main thread (this blocks forever);
    watcher.start(path_to_watch, syncManager);

    // cleanup (if watcher ever exists)
    networkManager.stopServer();
    if(networkThread.joinable()){
        networkThread.join();
    }
    return 0;
}