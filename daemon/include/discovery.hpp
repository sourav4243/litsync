#pragma once
#include <string>
#include <atomic>

class DiscoveryManager {
private:
    int udp_socket;
    std::atomic<bool> is_running;

public:
    DiscoveryManager();
    ~DiscoveryManager();

    // start shouting presence to the network
    void startBroadcasting(int tcp_port);
    void stopBroadcasting();
};