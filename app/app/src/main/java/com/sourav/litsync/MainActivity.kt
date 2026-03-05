package com.sourav.litsync

import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.provider.Settings
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity(){
    override fun onCreate(savedInstanceState: Bundle?){
        super.onCreate(savedInstanceState)

        setContent{
            MaterialTheme(colorScheme = darkColorScheme()){ // forcing dark theme for stealth UI
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ){
                    PermissionManager()
                }
            }
        }
    }

    @Composable
    fun PermissionManager(){
        // check if we already have permission
        var hasStoragePermission by remember{
            mutableStateOf(
                if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.R){
                    Environment.isExternalStorageManager()
                } else {
                    true    // for older android versions, standard manifest permission are enough
                }
            )
        }

        // create a launcher to handle user returning from the settings screen
        val storagePermissionLauncher = rememberLauncherForActivityResult(
            contract = ActivityResultContracts.StartActivityForResult()
        ) {
            // check again once user return
            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.R){
                hasStoragePermission = Environment.isExternalStorageManager()
            }
        }

        // the ui logic
        if(!hasStoragePermission){
            // show permission request screen
            Column(
                modifier = Modifier.fillMaxSize(),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement =  Arrangement.Center
            ) {
              Text(
                  text = "LitSync requires 'All Files Access' to sync your folder.",
                  textAlign = TextAlign.Center,
                  modifier = Modifier.padding(horizontal = 32.dp)
              )
                Spacer(modifier = Modifier.height(16.dp))
                Button(onClick = {
                    if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.R){
                        // fire the indent to open the specific settings page for the app
                        val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION)
                        intent.data = Uri.parse("package:${applicationContext.packageName}")
                        storagePermissionLauncher.launch(intent)
                    }
                }) {
                    Text("Grant Permission")
                }
            }
        } else {
            // state variables to update the UI
            var folderPath by remember { mutableStateOf("Initializing...") }
            var serverAddress by remember { mutableStateOf("Searching for Linux Server...") }
            val coroutineScope = rememberCoroutineScope()

            // LaunchedEffect runs background tasks when the screen loads
            LaunchedEffect(Unit){
                createLitSyncFolder()
                // start immortal background service
                startService(Intent(applicationContext, SyncService::class.java))

            }

            // we have permission! show placeholder for actual app ui
            Column(
                modifier = Modifier.fillMaxSize(),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.Center
            ) {
                Text(
                    text = "Permission Granted!",
                    style = MaterialTheme.typography.titleLarge
                )
                Spacer(modifier = Modifier.height(16.dp))
                Text(
                    text = "LitSync is running in the background.",
                    textAlign = TextAlign.Center,
                    color = MaterialTheme.colorScheme.primary,
                    modifier = Modifier.padding(horizontal = 32.dp)
                )
            }
        }
    }

    private fun createLitSyncFolder(): String {
        // point to root of user's internal storage, then append our folder name
//        val path = Environment.getExternalStorageDirectory().absolutePath + "/LitSync"
        val path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS).absolutePath + "/LitSync"
        val folder = java.io.File(path)

        if(!folder.exists()) {
            val success = folder.mkdirs()
            if(success) {
                println("[LitSync] Created master folder at: $path")
            } else {
                println("[LitSync] Failed to create folder!")
            }
        } else {
            println("[LitSync] Folder already exists at: $path")
        }
        return path
    }
}