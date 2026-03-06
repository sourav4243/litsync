#pragma once
#include <string>
#include <unordered_set>
#include <mutex>

class SyncManager {
private:
    std::unordered_set<std::string> ignored_files;
    std::mutex mtx;
    
    std::string active_client_ip;

public:
    void ignoreFile(const std::string& filename);
    void unignoreFile(const std::string& filename);
    bool isIgnored(const std::string& filename);

    void setClientIp(const std::string& ip);
    std::string getClientIp();
};