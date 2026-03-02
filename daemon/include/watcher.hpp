#pragma once
#include <string>
#include "sync_manager.hpp"
#include "network.hpp"  // so watcher access sendFile func.

class Watcher {
public:
    void start(const std::string& path_to_watch, SyncManager& syncManager, NetworkManager& networkManager, const std::string& target_ip, int target_port);
};