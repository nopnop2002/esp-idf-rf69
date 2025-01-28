# MQTT Example   
This is Radio and MQTT gateway application.   
```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
            |Publisher |--(MQTT)-->|  Broker  |--(MQTT)-->|  ESP32   |--(SPI)--->|   RF69   |==(Radio)==>
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+

            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
==(Radio)==>|   RF69   |--(SPI)--->|  ESP32   |--(MQTT)-->|  Broker  |--(MQTT)-->|Subscriber|
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```


# Configuration   
![Image](https://github.com/user-attachments/assets/35388735-4462-4a30-9e3a-936e0a1b84bd)
![Image](https://github.com/user-attachments/assets/c1767a6a-e171-4637-9f39-639302d390eb)

## WiFi Setting
![Image](https://github.com/user-attachments/assets/5353f2fd-be8e-4137-ab0b-0b9a2f0c91e8)

## Radio Setting

### MQTT to Radio   
 Subscribe with MQTT and send to Radio.   
 You can use mosquitto_pub as Publisher.   
 ```mosquitto_pub -h broker.emqx.io -p 1883 -t "/topic/radio/test" -m "test"```

```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
            |Publisher |--(MQTT)-->|  Broker  |--(MQTT)-->|  ESP32   |--(SPI)--->|   RF69   |==(Radio)==>
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```

![Image](https://github.com/user-attachments/assets/cd118950-9d0d-4d01-ad96-5110c9cd6819)

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\RadioHead69_RawDemo_RX   

### Radio to MQTT   
 Receive from Radio and publish as MQTT.   
 You can use mosquitto_sub as Subscriber.   
 ```mosquitto_sub -h broker.emqx.io -p 1883 -t "/topic/radio/test"```

```
            +----------+           +----------+           +----------+           +----------+
            |          |           |          |           |          |           |          |
==(Radio)==>|   RF69   |--(SPI)--->|  ESP32   |--(MQTT)-->|  Broker  |--(MQTT)-->|Subscriber|
            |          |           |          |           |          |           |          |
            +----------+           +----------+           +----------+           +----------+
```

![Image](https://github.com/user-attachments/assets/9d672fa5-131d-4fd3-a8f8-95ade4a22303)

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\RadioHead69_RawDemo_TX   

### Select Transport   
This project supports TCP,SSL/TLS,WebSocket and WebSocket Secure Port.   
- Using TCP Port.   
 TCP Port uses the MQTT protocol.   
 ![Image](https://github.com/user-attachments/assets/2df27b89-b69b-43bc-b688-83b1332396b2)

- Using SSL/TLS Port.   
 SSL/TLS Port uses the MQTTS protocol instead of the MQTT protocol.   
 ![Image](https://github.com/user-attachments/assets/d6f4f31e-6578-4860-b1b4-e4341c8ccd38)

- Using WebSocket Port.   
 WebSocket Port uses the WS protocol instead of the MQTT protocol.   
 ![Image](https://github.com/user-attachments/assets/c1767f30-a1ba-401e-87ea-3370a187c2ef)

- Using WebSocket Secure Port.   
 WebSocket Secure Port uses the WSS protocol instead of the MQTT protocol.   
 ![Image](https://github.com/user-attachments/assets/fdcd3170-b7bd-4455-a3b2-9dbf033f5c19)

__Note for using secure port.__   
The default MQTT server is ```broker.emqx.io```.   
If you use a different server, you will need to modify ```getpem.sh``` to run.   
```
chmod 777 getpem.sh
./getpem.sh
```

WebSocket/WebSocket Secure Port may differ depending on the broker used.   
If you use a different server, you will need to change the port number from the default.   

__Note for using MQTTS/WS/WSS transport.__   
If you use MQTTS/WS/WSS transport, you can still publish and subscribe using MQTT transport.   
```
+----------+                   +----------+           +----------+
|          |                   |          |           |          |
|  ESP32   | ---MQTTS/WS/WSS-->|  Broker  | ---MQTT-->|Subsctiber|
|          |                   |          |           |          |
+----------+                   +----------+           +----------+

+----------+                   +----------+           +----------+
|          |                   |          |           |          |
|  ESP32   | <--MQTTS/WS/WSS---|  Broker  | <--MQTT---|Publisher |
|          |                   |          |           |          |
+----------+                   +----------+           +----------+
```



### Specifying an MQTT Broker   
You can specify your MQTT broker in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```mqtt-broker.local```   
- Fully Qualified Domain Name   
 ```broker.emqx.io```

You can use this as broker.   
https://github.com/nopnop2002/esp-idf-mqtt-broker

### Secure Option
Specifies the username and password if the server requires a password when connecting.   
[Here's](https://www.digitalocean.com/community/tutorials/how-to-install-and-secure-the-mosquitto-mqtt-messaging-broker-on-debian-10) how to install and secure the Mosquitto MQTT messaging broker on Debian 10.   
![config-mqtt-5](https://github.com/user-attachments/assets/58555299-f9f0-424f-9d2f-a76b6fbe8da7)
