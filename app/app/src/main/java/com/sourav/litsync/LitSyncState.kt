package com.sourav.litsync

import kotlinx.coroutines.flow.MutableStateFlow

// a global singleton object that both ui and background service can see
object LitSyncState {
    val folderPath = MutableStateFlow("Initializing...")
    val serverAddress = MutableStateFlow("Searching for Linux Server...")
}