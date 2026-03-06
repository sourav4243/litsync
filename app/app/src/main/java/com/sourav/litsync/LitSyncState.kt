package com.sourav.litsync

import kotlinx.coroutines.flow.MutableStateFlow
import java.util.Collections

// a global singleton object that both ui and background service can see
object LitSyncState {
    val folderPath = MutableStateFlow("Initializing...")
    val serverAddress = MutableStateFlow("Searching for Linux Server...")

    // list to keep track of files currently being downloaded so watcher ignore them
    val filesBeingReceived: MutableSet<String> = Collections.synchronizedSet(mutableSetOf())
}