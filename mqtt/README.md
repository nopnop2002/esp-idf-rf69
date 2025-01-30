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
![Image](https://github.com/user-attachments/assets/3a55cceb-7e5d-4cd7-8084-e7e081361fc6)

## WiFi Setting
Set the information of your access point.   
![Image](https://github.com/user-attachments/assets/e8a79ecc-4542-4ea3-a272-e8f6290cba88)

## Radio Setting
![Image](https://github.com/user-attachments/assets/02c78d35-6b42-4c1b-9a64-9062356d4348)

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

Communicate with Arduino Environment.   
Run this sketch.   
ArduinoCode\RadioHead69_RawDemo_TX   

## Broker Setting
Set the information of your MQTT broker.   

### Specifying an MQTT Broker   
You can specify your MQTT broker in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```mqtt-broker.local```   
- Fully Qualified Domain Name   
 ```broker.emqx.io```

### Select Transport   
This project supports TCP,SSL/TLS,WebSocket and WebSocket Secure Port.   
![Image](https://github.com/user-attachments/assets/7840d961-bbcc-44d7-a509-4f774cd6ec2c)

- Using TCP Port.   
 TCP Port uses the MQTT protocol.   

- Using SSL/TLS Port.   
 SSL/TLS Port uses the MQTTS protocol instead of the MQTT protocol.   

- Using WebSocket Port.   
 WebSocket Port uses the WS protocol instead of the MQTT protocol.   

- Using WebSocket Secure Port.   
 WebSocket Secure Port uses the WSS protocol instead of the MQTT protocol.   

__Note for using secure port.__   
The default MQTT server is ```broker.emqx.io```.   
If you use a different server, you will need to modify ```getpem.sh``` to run.   
```
chmod 777 getpem.sh
./getpem.sh
```

WebSocket/WebSocket Secure Port may differ depending on the broker used.   
If you use a different MQTT server than the default, you will need to change the port number from the default.   

### Select MQTT Protocol   
This project supports MQTT Protocol V3.1.1/V5.   
![Image](https://github.com/user-attachments/assets/e4192ebd-8692-4f5d-82ae-287cfd0441c1)

### Enable Secure Option
Specifies the username and password if the server requires a password when connecting.   
[Here's](https://www.digitalocean.com/community/tutorials/how-to-install-and-secure-the-mosquitto-mqtt-messaging-broker-on-debian-10) how to install and secure the Mosquitto MQTT messaging broker on Debian 10.   
![Image](https://github.com/user-attachments/assets/7e9a2297-9010-49b0-80ff-06e41f179af5)
