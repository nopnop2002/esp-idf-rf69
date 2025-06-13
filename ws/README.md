# HTTP Example   
This is RF69 and WebSocket gateway application.   
```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            | WS Client |-(Socket)->|   ESP32   |--(SPI)--->|    RF69   |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+

            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|    RF69   |--(SPI)--->|   ESP32   |-(Socket)->| WS Server |
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```


# Configuration
![Image](https://github.com/user-attachments/assets/3576da07-5ee1-404d-aad4-fd6424d2ea12)
![Image](https://github.com/user-attachments/assets/1602d47e-5452-4bf4-8038-fa8663f0ab67)

## WiFi Setting
Set the information of your access point.   
![Image](https://github.com/user-attachments/assets/dc45ce85-2a4c-4f81-9e15-6d49c190840a)

## Radioi Setting
Set the wireless communication direction.   

### WS to Radio
Subscribe with WebSocket and send to Radio.   
ESP32 acts as WebSocket Server.   
You can use ws-client.py as WS Client.   
```python3 ws-client.py```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
            | WS Client |-(Socket)->|   ESP32   |--(SPI)--->|    RF69   |==(Radio)==>
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![Image](https://github.com/user-attachments/assets/e8cfaf3a-f97e-4d07-9c0f-c561fde4eaf2)

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\RadioHead69_RawDemo_RX   


### Radio to WS
Receive from Radio and publish as WebSocket.   
ESP32 acts as WebSocket Client.   
Use [this](https://components.espressif.com/components/espressif/esp_websocket_client) component.   
You can use ws-server.py as WS Server.   
```python3 ws-server.py```

```
            +-----------+           +-----------+           +-----------+
            |           |           |           |           |           |
==(Radio)==>|    RF69   |--(SPI)--->|   ESP32   |-(Socket)->| WS Server |
            |           |           |           |           |           |
            +-----------+           +-----------+           +-----------+
```

![Image](https://github.com/user-attachments/assets/3fc6154b-0a33-4813-adfd-440ab44feea1)

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\RadioHead69_RawDemo_TX   


### Specifying an WebSocket Server   
You can specify your WebSocket Server in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```ws-server.local```   
- Fully Qualified Domain Name   
 ```ws-server.public.io```


