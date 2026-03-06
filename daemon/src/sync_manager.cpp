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

void SyncManager::setClientIp(const std::string& ip){
    std::lock_guard<std::mutex> lock(mtx);
    active_client_ip = ip;
}

std::string SyncManager::getClientIp(){
    std::lock_guard<std::mutex> lock(mtx);
    return active_client_ip;
}