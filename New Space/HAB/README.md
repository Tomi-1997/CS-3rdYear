# Long Range High Altitude Balloon

### Payload 

A CubeCell GPS unit. <br>
![D](https://github.com/Tomi-1997/CS-3rdYear/blob/main/New%20Space/HAB/pics/Payload.jpg) <br>
![D](https://github.com/Tomi-1997/CS-3rdYear/blob/main/New%20Space/HAB/pics/Disassembled.jpeg) <br>
![D](https://github.com/Tomi-1997/CS-3rdYear/blob/main/New%20Space/HAB/pics/Package.jpeg) <br>
### Groundstation
We used an ESP32 Wireless Stick to receive info from the payload and write to file. <br>
It uploaded the file to this [github repository](https://github.com/sonyamonyaa/SpaceLog/tree/main) every half an hour. <br>
### Launch
![D](https://github.com/Tomi-1997/CS-3rdYear/blob/main/New%20Space/HAB/pics/Launch.JPG) <br>
### Results
The connectionion between the payload and the ground station was stable. <br> 
Some kilometers after crossing the border to Jordan the GPS unit seems to have stopped updating. <br>
The sent packet had a decreasing signal strength, but the gps information was constant. <br>
As the rssi signal keep decreasing, we assume the unit kept floating. <br>

![D](https://github.com/Tomi-1997/CS-3rdYear/blob/main/New%20Space/HAB/pics/Rssi_VS_Alt.jpeg) <br>
![D](https://github.com/Tomi-1997/CS-3rdYear/blob/main/New%20Space/HAB/pics/Trajectory.jpg) <br>

### Pre-launch CubeCell Tasks
> **legend** <br> 
✅ - done <br>
🔳 - needs to be done<br>
💬 - in progress<br>

1 ✅ Display <br>
2 ✅ LoRa <br>
3 ✅ GPS <br>
4 ✅ Servo <br>
5 ✅ Send data from CubeCell to Wireless Stick via Lora <br>
6 ✅ Battery drainer <br>
7 ✅ [Payload] State machine- calcualte ascent rate <br>
8 ✅ [Ground station] Standby from payload, print packets from serial to file <br>
7 ✅ [Payload] Launch <br>
8 ✅ [Ground station] Convert LoRa stream from payload to local file <br>
