# HTTP Example   
This is RF69 and HTTP gateway application.   
```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            |HTTP Client|--(HTTP)-->|   ESP32   |--(SPI)--->|   RF69    |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+

            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|   RF69    |--(SPI)--->|   ESP32   |--(HTTP)-->|HTTP Server|
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```



# Configuration
![Image](https://github.com/user-attachments/assets/104c96c3-406c-474d-9978-fd470b42102e)
![Image](https://github.com/user-attachments/assets/e101c4b4-52de-495d-9091-239ddc1e6b81)

## WiFi Setting
Set the information of your access point.   
![Image](https://github.com/user-attachments/assets/14626be6-4450-4bb3-8cfb-38ed8d273ab9)

## Radioi Setting
Set the wireless communication direction.   

### HTTP to Radio
Subscribe with HTTP and send to Radio.   
ESP32 acts as HTTP Server.   
You can use curl as HTTP Client.   
```sh ./http-client.sh```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            |HTTP Client|--(HTTP)-->|   ESP32   |--(SPI)--->|   RF69    |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![Image](https://github.com/user-attachments/assets/3275e7a9-a8db-4b84-a960-3d1746e2f247)

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-rf69/tree/main/ArduinoCode/RadioHead69_RawDemo_RX).   

### Radio to HTTP
Receive from Radio and publish as HTTP.   
ESP32 acts as HTTP Client.   
You can use nc(netcat) as HTTP Server.   
```sh ./http-server.sh```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|   RF69    |--(SPI)--->|   ESP32   |--(HTTP)-->|HTTP Server|
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![Image](https://github.com/user-attachments/assets/f650ae3d-09c3-48d4-9c5f-00d9795ef424)

Communicate with Arduino Environment.   
I tested it with [this](https://github.com/nopnop2002/esp-idf-rf69/tree/main/ArduinoCode/RadioHead69_RawDemo_TX).   

### Specifying an HTTP Server   
You can specify your HTTP Server in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```http-server.local```   
- Fully Qualified Domain Name   
 ```http-server.public.io```


