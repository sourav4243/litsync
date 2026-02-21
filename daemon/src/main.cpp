#include<iostream>
#include<sys/inotify.h>
#include<unistd.h>
#include<filesystem>
#include<string>

// inotify gives us a stream of bytes. We need a buffer large enough to hold multiple events and the variable-length filenames attached to them.
#define MAX_EVENTS 1024
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (MAX_EVENTS * (EVENT_SIZE * 16))

int main(){
    // 1. set up the directory we want LitSync to watch
    std::string path_to_watch = "./litsync_folder";
    std::filesystem::create_directory(path_to_watch);

    // 2. Initialize the inotify instance
    int fd = inotify_init();
    if(fd<0){
        std::cerr<<"Fatal: failed to initialize inotify.\n";
        return 1;
    }

    // 3. Add a watch to our directory to Create, Delete, and Modify events
    int wd = inotify_add_watch(fd, path_to_watch.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    if(wd==-1){
        std::cerr<<"Fatal: could not watch directory: "<<path_to_watch<<std::endl;
        return -1;
    }

    std::cout<<"LitSync daemon started.\n";
    std::cout<<"Watching  directory: "<<path_to_watch<<std::endl;
    std::cout<<"Try opening another terminal and touching/deleting files in that folder.\n\n";

    char buffer[EVENT_BUF_LEN];

    // 4. Main event loop
    while(true){
        // This read() call blocks. It waits patiently with 0% CPU usage until the kernel tells it a file changed.
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if(length<0){
            std::cerr<<"Error reading inotify events.\n";
            break;
        }

        // Parse the raw bytes into inotify_event structs
        int i=0;
        while(i<length){
            struct inotify_event* event = (struct inotify_event*) &buffer[i];

            if(event->len){
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

            // Move the pointer to next event in the buffer
            i+=EVENT_SIZE + event->len;
        }
    }

    // Cleanup (though we have an infinite loop above right now)
    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}