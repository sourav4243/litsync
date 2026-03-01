#include "sync_manager.hpp"

void SyncManager::ignoreFile(const std::string& filename){
    std::lock_guard<std::mutex> lock(mtx);
    ignored_files.insert(filename);
}

void SyncManager::unignoreFile(const std::string& filename){
    std::lock_guard<std::mutex> lock(mtx);
    ignored_files.erase(filename);
}

bool SyncManager::isIgnored(const std::string& filename){
    std::lock_guard<std::mutex> lock(mtx);
    return ignored_files.find(filename) != ignored_files.end();
}