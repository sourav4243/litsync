package com.sourav.litsync

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.net.DatagramPacket
import java.net.DatagramSocket

class DiscoveryManager {
    // use "suspend" function so this heavy network loop runs on a background thread (Dispatchers.IO) without freezing ui
    suspend fun listenForServer(onServerFound: (String, Int) -> Unit){
        withContext(Dispatchers.IO){
            var socket: DatagramSocket? = null
            try {
                // listen on port C++ daemon is shouting at
                socket = DatagramSocket(8888)
                socket.broadcast = true

                val buffer = ByteArray(1024)
                val packet = DatagramPacket(buffer, buffer.size)

                println("[Discovery] Listening for UDP broadcasts on port 8888...")

                // keep listening until we catch the beacon
                while(true){
                    socket.receive(packet)
                    val message = String(packet.data, 0, packet.length).trim()

                    // check if it's our LitSync C++ daemon
                    if(message.startsWith("LITSYNC_SERVER|")){
                        val serverIp = packet.address.hostAddress ?: "Unknown IP"
                        val tcpPort = message.split("|")[1].toInt()

                        println("[Discovery] found Server! IP: $serverIp, Port: $tcpPort")

                        // back to main ui thread to update screen
                        withContext(Dispatchers.Main){
                            onServerFound(serverIp, tcpPort)
                        }

                        // for now, stop listening once we find it
                        break
                    }
                }
            } catch (e : Exception){
                println("[Discovery] Error: ${e.message}")
            } finally {
                socket?.close()
            }
        }
    }
}