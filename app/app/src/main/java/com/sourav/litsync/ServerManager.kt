package com.sourav.litsync

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.isActive
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileOutputStream
import java.net.ServerSocket
import java.net.Socket

class ServerManager(private val folderPath: String) {
    suspend fun startListening(port: Int = 8081){
        withContext(Dispatchers.IO){
            try{
                ServerSocket(port).use{ serverSocket ->
                    println("[ServerManager] Android Server listening for pushes on port $port...")

                    while(isActive){
                        val client = serverSocket.accept()  // pauses here until linux connects
                        handleClient(client)
                    }
                }
            }catch(e:Exception){
                println("[ServerManager] Server crashed: ${e.message}")
            }
        }
    }

    private fun handleClient(socket: Socket){
        try {
            socket.use{
                val inputStream = it.getInputStream()

                // read protocol header exactly up to the newline char '\n'
                val headerBuilder = StringBuilder()
                var b: Int
                while(true){
                    b = inputStream.read()
                    if(b == -1 || b == '\n'.code) break
                    headerBuilder.append(b.toChar())
                }

                val header = headerBuilder.toString()
                val parts = header.split("|")

                if(parts.size >= 3){
                    val action = parts[0]
                    val fileName = parts[1]
                    val size = parts[2].toLongOrNull() ?: 0L

                    val targetFile = File(folderPath, fileName)

                    when(action){
                        "UPLOAD" -> {
                            println("[ServerManager] Receiving file from linux: $fileName ($size bytes)")

                            // tell watcher to ignore this file
                            LitSyncState.filesBeingReceived.add(fileName)

                            // stream raw file bytes directly to android hard drive
                            FileOutputStream(targetFile).use{ fos ->
                                val buffer = ByteArray(4096)
                                var bytesRead: Int
                                var totalRead = 0L

                                while(totalRead < size){
                                    // only read exactly what we need so we don't block forever
                                    val toRead = minOf(buffer.size.toLong(), size - totalRead).toInt()
                                    bytesRead = inputStream.read(buffer, 0, toRead)
                                    if(bytesRead == -1) break

                                    fos.write(buffer, 0, bytesRead)
                                    totalRead += bytesRead
                                }
                            }

                            // file saved. unignore now
                            LitSyncState.filesBeingReceived.remove(fileName)
                            println("[ServerManager] Successfully saved linux push: $fileName")
                        }

                        "DELETE" -> {
                            if(targetFile.exists()){
                                // tell watcher to ignore this file
                                LitSyncState.filesBeingReceived.add(fileName)

                                targetFile.delete()
                                println("[ServerManager] Deleted file via linux command: $fileName")

                                // unignore file
                                LitSyncState.filesBeingReceived.remove(fileName)
                            }
                        }
                    }
                }
            }
        } catch (e: Exception){
            println("[ServerManager] Client handling failed: ${e.message}")
        }
    }
}