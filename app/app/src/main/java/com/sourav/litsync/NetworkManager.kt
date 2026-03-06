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
                    // must end with \n as C++ uses std::getline
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

    suspend fun sendDeleteCommand(fileName: String, targetIp: String, targetPort: Int): Boolean {
        return withContext(Dispatchers.IO){
            try{
                println("[Network] Sending DELETE command for $fileName to $targetIp:$targetPort")

                // open the TCP socket to the linux daemon
                Socket(targetIp, targetPort).use { socket ->
                    val outputStream = socket.getOutputStream()

                    // construct and send a LitSync Protocol Delete Header
                    val header = "DELETE|$fileName|0\n"
                    outputStream.write(header.toByteArray())
                    outputStream.flush()

                    println("[Network] Successfully commanded Linux to delete: $fileName")
                    true
                }
            } catch (e: Exception) {
                println("[Network] Delete command failed: ${e.message}")
                false
            }
        }
    }

    suspend fun sendPing(targetIp: String, targetPort: Int): Boolean{
        return withContext(Dispatchers.IO){
            try{
                // a strict 2-sec timeout. if takes longer, connection is dead
                java.net.Socket().use { socket ->
                    socket.connect(java.net.InetSocketAddress(targetIp, targetPort), 2000)

                    val outputStream = socket.getOutputStream()
                    // a dummy heartbeat
                    outputStream.write("PING|heartbeat|0\n".toByteArray())
                }
                true
            } catch (e: Exception) {
                // socketTimeoutExecution or ConnectException means the daemon is offline
                false
            }
        }
    }
}