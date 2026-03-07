<!-- # LitSync (WIP)

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
- Communicates directly with the C++ backend through a socket layer   -->

# ⚡ LitSync

[![Download APK](https://img.shields.io/badge/Download-APK-green.svg?style=for-the-badge&logo=android)](https://github.com/sourav4243/litsync/releases/latest)

A blazingly fast, true two-way local file synchronization engine between Linux and Android. Built entirely from scratch in C++ and Kotlin, LitSync completely eliminates the need for USB cables, cloud storage, or messaging yourself files on WhatsApp.

If your phone and your Linux machine are on the same Wi-Fi network, your files are instantly synced. 

## Features
* **True Two-Way Sync:** Drop a file on your PC, it appears on your phone. Save a file on your phone, it appears on your PC.
* **Auto-Discovery:** Zero manual IP configuration. The daemon and app find each other instantly via UDP broadcasting.
* **Dynamic Network Tracking:** Silent 3-second heartbeats keep the connection alive and track phone IP changes dynamically.
* **Resilient Background Engine:** An Android Foreground Service ensures files keep syncing even when the app is closed.
* **Custom TCP Protocol:** Built on bare-metal sockets (`UPLOAD|filename|filesize`) for maximum local network transfer speeds.
* **Cross-Version Android Support:** Gracefully handles modern Android 12+ "All Files Access" as well as Android 10 "Legacy Storage" restrictions.

---

##  Getting Started

### 1. Clone the Repository
Open your Linux terminal and clone the project:
```bash
git clone https://github.com/sourav4243/litsync.git
cd litsync
```

### 2. Linux Daemon Setup (C++)
The Linux daemon acts as the backbone of the system, watching your local folder and handling incoming connections.

**Compilation:**
Navigate to the daemon directory and build the project using CMake. *(Ensure you have `cmake` and a modern `g++` compiler installed).*
```bash
cd daemon
mkdir build
cd build
cmake ..
make
```

**Running the Daemon:**
```bash
./litsync
```
*The daemon will automatically create a `litsync_folder` in your build directory and begin broadcasting its presence on port 8888.*

### 3. Android App Setup (Kotlin)
The Android client listens for the Linux daemon, tracks local file changes, and maintains the background connection.

**Option A: Easiest Way (Direct Download):**
1. Go to the `Releases page` and download the latest .apk file to your Android phone.
2. Open the file to install it. (You may need to allow "Install unknown apps" from your browser or file manager).

**Option B: For Developers (Build from Source)**
1. Open the `app/` folder in **Android Studio**.
2. Connect your Android device via USB debugging or Wireless debugging.
3. Click **Run** (`Shift + F10`) to build and install the APK onto your device.


**Permissions (Crucial):** Upon opening the app for the first time, you **must** grant it 
1. Storage permissions so it can read and write files. 
    * **Android 11+:** The app will prompt you to "Allow management of all files".
    * **Android 10 & below:** If the prompt does not appear, go to your phone's `Settings -> Apps -> LitSync -> Permissions` and manually grant **Storage** access. 
2. Notification Permissoins
---

## How to Use

Once both systems are running, the magic is completely invisible.

1. Ensure both your Linux machine and Android phone are on the **same Wi-Fi network** (or use your phone as a mobile hotspot for the PC).
2. Start the Linux daemon (`./litsync`).
3. Open the LitSync app on your phone. You will see the UI update to show it has connected to the Linux server.
4. **Test the Sync:**
   * **Linux -> Android:** Drop any photo, document, or video into the `litsync_folder` on your PC. Check your phone's `Documents/LitSync` folder—it's already there.
   * **Android -> Linux:** Save or move a file into your phone's `Documents/LitSync` folder. Watch your Linux terminal instantly report the successful transfer and save it to your PC.

---

## Future Enhancements
LitSync's core engine is fully functional, but development is ongoing. Upcoming features include:
- **Initial State "Catch-Up" Sync:** Synchronize files that were added while devices were offline.
- **Recursive Subdirectory Support:** Watch and sync nested folders, not just the root directory.
- **Real-Time UI Progress Bars:** Display exact transfer percentages for large files.
- **Boot Receiver:** Automatically launch the Android SyncService when the phone turns on.