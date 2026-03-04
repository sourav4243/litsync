package com.sourav.litsync

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileInputStream
import java.net.Socket

class NetworkManager {
    suspend fun sendFile(file: File, targetIp: String, targetPort: Int): Boolean {
        return withContext(Dispatchers.IO){
            try{
                println("[Network] Connecting to $targetIp:$targetPort...")

                // open the TCP socket to the linux daemon
                Socket(targetIp, targetPort).use { socket ->
                    val outputStream = socket.getOutputStream()

                    // construct and send LitSync Protocol Header
                    // must end with \n as C++ uses std::getling
                    val header = "UPLOAD|${file.name}|${file.length()}\n"
                    outputStream.write(header.toByteArray())

                    // stream the file bytes efficiently in 4KB chunks
                    FileInputStream(file).use { fileInputStream ->
                        val buffer = ByteArray(4096)
                        var bytesRead: Int
                        while(fileInputStream.read(buffer).also { bytesRead = it } != -1){
                            outputStream.write(buffer, 0, bytesRead)
                        }
                    }

                    outputStream.flush()
                    println("[Network] Successfully sent: ${file.name}")
                    true
                }
            } catch (e: Exception) {
                println("[Network] Upload failed: ${e.message}")
                false
            }
        }
    }
}