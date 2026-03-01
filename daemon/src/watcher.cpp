#include "watcher.hpp"
#include <iostream>
#include <sys/inotify.h>
#include <unistd.h>

#define MAX_EVENTS 1024
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (MAX_EVENTS * (EVENT_SIZE * 16))

void Watcher::start(const std::string& path_to_watch, SyncManager& syncManager){
    int fd = inotify_init();
    if(fd < 0){
        std::cerr << "[Watcher] Fatal: failed to initialize inotify.\n";
        return;
    }

    int wd = inotify_add_watch(fd, path_to_watch.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE);
    if(wd == -1){
        std::cerr << "[Watcher] Fatal: could not watch directory: " << path_to_watch << std::endl;
        return;
    }

    std::cout << "[Watcher] Watcher started on directory: " << path_to_watch << std::endl;
    char buffer[EVENT_BUF_LEN];

    while(true){
        // This read() call blocks. It waits patiently with 0% CPU usage until the kernel tells it a file changed.
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if(length < 0){
            std::cerr << "[Watcher] Error reading inotify events.\n";
            break;
        }

        int i=0;
        while(i < length){
            struct inotify_event* event = (struct inotify_event*) &buffer[i];

            if(event->len){
                std::string filename = event->name;

                // Check the SyncManager to see if the netwrok thread is currently downloading this file.
                if(syncManager.isIgnored(filename)){
                    // if OS tells us the file is completely closed, the network download is finished!
                    if(event->mask & IN_CLOSE_WRITE){
                        std::cout << "[Watcher] Network finished writing, unlocking: " << filename << std::endl;
                        syncManager.unignoreFile(filename);
                    }
                    i += EVENT_SIZE + event->len;
                    continue;
                }

                if(event->mask & IN_CREATE){
                    std::cout<<"[+] File Created: "<<event->name<<std::endl;
                } else if(event->mask & IN_DELETE || event->mask & IN_MOVED_FROM){
                    std::cout<<"[-] File Deleted: "<<event->name<<std::endl;
                } else if(event->mask & IN_MODIFY){
                    std::cout<<"[*] File Modified: "<<event->name<<std::endl;
                } else if(event->mask & IN_MOVED_TO){
                    std::cout<<"[*] File Moved In: "<<event->name<<std::endl;
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}