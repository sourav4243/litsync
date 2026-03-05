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
        serviceScope.launch{
            // get folder path
            val folderPath = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS).absolutePath + "/LitSync"
            val folder = File(folderPath)
            if (!folder.exists()) folder.mkdirs()

            LitSyncState.folderPath.value = folderPath

            // discover linux server
            val discovery = DiscoveryManager()
            discovery.listenForServer { ip, port ->

                LitSyncState.serverAddress.value = "$ip:$port"

                // start watching for file changes
                activeWatcher = Watcher(folderPath) {action, file ->
                    serviceScope.launch{
                        if(action == "UPLOAD"){
                            NetworkManager().sendFile(file, ip, port)
                        }else if(action == "DELETE"){
                            NetworkManager().sendDeleteCommand(file.name, ip, port)
                        }
                    }
                }

                activeWatcher?.startWatching()
                println("[SyncService] Headless Engine activity monitering $folderPath targeting $ip:$port")
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