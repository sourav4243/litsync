package com.sourav.litsync

import android.os.FileObserver
import java.io.File

@Suppress("DEPRECATION") // suppressing because android preferred API 29+ methods, but we want this to work on API 26+
class Watcher(
    private val directoryPath: String,
    private val onFileChange: (action: String, file: File) -> Unit
) : FileObserver(directoryPath, CLOSE_WRITE or MOVED_TO or DELETE or MOVED_FROM) {

    override fun onEvent(event: Int, path: String?) {
        // android sometimes fires ghost events with null paths.. ignore them
        if(path == null) return

        val affectedFile = File(directoryPath, path)

        // android's FileObserver uses bitmasks, so we isolate the core event type
        when(event and ALL_EVENTS){

            CLOSE_WRITE, MOVED_TO -> {
                println("[Watcher] File completely written: $path")
                onFileChange("UPLOAD", affectedFile)
            }
            DELETE, MOVED_FROM -> {
                println("[Watcher] File removed: $path")
                onFileChange("DELETE", affectedFile)
            }
        }
    }
}