# Basic Example
Send data from primary to secondary.   
In the secondary, the characters are converted and sent back.   

- ESP32 is Primary   
```
+-----------+           +-----------+             +-----------+           +-----------+
|           |           |           |             |           |           |           |
|  Primary  |===(SPI)==>|   RF69    |---(Radio)-->|   RF69    |===(SPI)==>| Secondary |
|   ESP32   |           |           |             |           |           |           |
|           |           |           |             |           |           |           |
|           |<==(SPI)===|           |<--(Radio)---|           |<==(SPI)===|           |
|           |           |           |             |           |           |           |
+-----------+           +-----------+             +-----------+           +-----------+
```

- ESP32 is Secondary   

```
+-----------+           +-----------+             +-----------+           +-----------+
|           |           |           |             |           |           |           |
|  Primary  |===(SPI)==>|   RF69    |---(Radio)-->|   RF69    |===(SPI)==>| Secondary |
|           |           |           |             |           |           |   ESP32   |
|           |           |           |             |           |           |           |
|           |<==(SPI)===|           |<--(Radio)---|           |<==(SPI)===|           |
|           |           |           |             |           |           |           |
+-----------+           +-----------+             +-----------+           +-----------+
```


# Configuration
![config-app-1](https://user-images.githubusercontent.com/6020549/168983261-c258d86b-09dc-4d4f-88dd-f4510c8b8280.jpg)
![Image](https://github.com/user-attachments/assets/4b6b4954-f595-44e5-b97f-074ba7d8f9cc)

# Communication with the Arduino environment   
- ESP32 is the primary   
I tested it with [this](https://github.com/nopnop2002/esp-idf-rf69/tree/main/ArduinoCode/RadioHead69_RawDemo_PONG).   

- ESP32 is the secondary   
I tested it with [this](https://github.com/nopnop2002/esp-idf-rf69/tree/main/ArduinoCode/RadioHead69_RawDemo_PING).
