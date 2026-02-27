#pragma once
#include <string>
#include <unordered_set>
#include <mutex>

class SyncState {
private:
    std::unordered_set<std::string>ignored_files;
    std::mutex mtx;

public:
    void ignoreFile(const std::string& filename){
        std::lock_guard<std::mutex>lock(mtx);
        ignored_files.insert(filename);
    }

    void unignoreFile(const std::string& filename){
        std::lock_guard<std::mutex>lock(mtx);
        ignored_files.erase(filename);
    }

    bool isIgnored(const std::string& filename){
        std::lock_guard<std::mutex>lock(mtx);
        return ignored_files.find(filename) != ignored_files.end();
    }
};