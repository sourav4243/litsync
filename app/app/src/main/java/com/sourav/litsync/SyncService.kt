package com.sourav.litsync

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Environment
import android.os.IBinder
import androidx.core.app.NotificationCompat
import kotlinx.coroutines.*
import java.io.File

class SyncService : Service() {
    private val CHANNEL_ID = "LitSyncChannel"

    // give the service its own background thread pool
    private val serviceScope = CoroutineScope(Dispatchers.IO + SupervisorJob())
    private var activeWatcher: Watcher? = null

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        // build a permanent notification
        val notification: Notification = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("LitSync Engine Active")
            .setContentText("Monitoring folder & syncing files...")
            .setSmallIcon(android.R.drawable.ic_popup_sync) // built-in android sync icon
            .setPriority(NotificationCompat.PRIORITY_LOW)   // low priority so it doesn't vibrate/beep
            .setOngoing(true)   // cannot be swiped away by the user
            .build()

        // anchor service to the background
        startForeground(1, notification)

        println("[SyncService] Foreground Service Started!")

        // start the engine directly inside the service.
        startSyncEngine()

        // START_STICKY tells android, "if android run out of RAM and kill the service, reboot the service immediately"
        return START_STICKY
    }

    private fun startSyncEngine(){
        serviceScope.launch {
            // get folder path
            val folderPath = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS).absolutePath + "/LitSync"
            val folder = File(folderPath)
            if (!folder.exists()) folder.mkdirs()

            LitSyncState.folderPath.value = folderPath

            // start Android TCP server in background (for receiving files)
            val serverManager = ServerManager(folderPath)
            launch{
                serverManager.startListening(8081)
            }

            while (isActive) {
                LitSyncState.serverAddress.value = "Searching for Linux Server..."
                println("[SyncService] Searching for Linux Daemon...")

                // create a stop sign to wait for the result
                val serverFound = CompletableDeferred<Pair<String, Int>>()

                // coroutine pauses here until it hears a UDP broadcast
                val discovery = DiscoveryManager()
                discovery.listenForServer { ip, port ->
                    // when server replies, complete the stop sign with the data
                    serverFound.complete(Pair(ip, port))
                }

                // code freezes here until serverFound is completed
                val result = serverFound.await()
                val currentIp = result.first
                val currentPort = result.second

                // now it's connected
                LitSyncState.serverAddress.value = "$currentIp:$currentPort"

                // a tripwire to trigger if network dies
                val connectionLost = CompletableDeferred<Unit>()

                // start continuous heartbeat ping
                val heartbeatJob = launch {
                    while (isActive) {
                        kotlinx.coroutines.delay(3000)  // ping every 3 seconds
                        val isAlive = NetworkManager().sendPing(currentIp, currentPort)
                        if (!isAlive) {
                            println("[SyncService] Heartbeat lost! Tripping wire...")
                            connectionLost.complete(Unit)   // trip the wire
                            break
                        }
                    }
                }

                // start watching for file changes
                activeWatcher = Watcher(folderPath) { action, file ->
                    serviceScope.launch {
                        val success = if (action == "UPLOAD") {
                            NetworkManager().sendFile(file, currentIp, currentPort)
                        } else if (action == "DELETE") {
                            NetworkManager().sendDeleteCommand(file.name, currentIp, currentPort)
                        } else {
                            true
                        }

                        if(!success){
                            println("[SyncService] Network request failed! Tripping wire...")
                            connectionLost.complete(Unit)   // trip the wire
                        }
                    }
                }

                activeWatcher?.startWatching()
                println("[SyncService] Connected! Monitering $folderPath targeting $currentIp:$currentPort")

                // pause main loop here forever UNTIL the tripwire is triggered
                connectionLost.await()

                // disconnected
                // clean up the threads before the while loop starts over
                println("[SyncService] Connection lost. Restarting search.")
                heartbeatJob.cancel()
                activeWatcher?.stopWatching()
                activeWatcher = null
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        println("[SyncService] Engine Shutting Down...")
        activeWatcher?.stopWatching()
        serviceScope.cancel()   // clean up background threads
    }

    override fun onBind(intent: Intent?): IBinder? {
        // it is a "Started" service, not a "Bound" service, so return null
        return null
    }

    private fun createNotificationChannel(){
        // android 8.0+ required notification channels
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O){
            val channel = NotificationChannel(
                CHANNEL_ID,
                "LitSync Background Engine",
                NotificationManager.IMPORTANCE_LOW
            )
            val manager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            manager.createNotificationChannel(channel)
        }
    }
}