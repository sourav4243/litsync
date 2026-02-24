# LitSync (WIP)

A real-time file synchronization system designed for seamless communication between a Linux machine and an Android device over a local network.

It consists of:
- A high-performance C++ daemon running on Linux  
- A Kotlin-based Android app acting as a sync client  

The system automatically discovers devices on the same network, monitors file changes, and synchronizes them.

---

## Architecture Overview

LitSync follows a client–daemon model.

### Linux Machine
- Runs a multithreaded C++ daemon  
- Watches directories using inotify  
- Broadcasts availability via mDNS  
- Sends and receives raw file bytes over TCP sockets  

### Android Device
- Discovers the daemon using Android NSD  
- Monitors selected folders  
- Runs a foreground synchronization service  
- Communicates directly with the C++ backend through a socket layer  