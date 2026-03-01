#pragma once
#include <string>
#include <unordered_set>
#include <mutex>

class SyncManager {
private:
    std::unordered_set<std::string> ignored_files;
    std::mutex mtx;

public:
    void ignoreFile(const std::string& filename);
    void unignoreFile(const std::string& filename);
    bool isIgnored(const std::string& filename);
};