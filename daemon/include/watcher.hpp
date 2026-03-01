#pragma once
#include <string>
#include "sync_manager.hpp"

class Watcher {
public:
    void start(const std::string& path_to_watch, SyncManager& syncManager);
};