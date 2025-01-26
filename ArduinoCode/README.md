# Example of Arduino environment
I used [this](https://github.com/adafruit/RadioHead) library.   

# Wireing   
|RFM69||ATmega328|ESP8266|
|:-:|:-:|:-:|:-:|
|MISO|--|GPIO12|GPIO12|
|SCK|--|GPIO13|GPIO14|
|MOSI|--|GPIO11|GPIO13|
|NSS|--|GPIO4|GPIO2|
|DIO0|--|GPIO3|GPIO15|
|RESET|--|GPIO2|GPIO16|
|GND|--|GND|GND|
|VCC|--|3.3V(*1)|3.3V|

(*1)   
UNO's 3.3V output can only supply 50mA.   
In addition, the output current capacity of UNO-compatible devices is smaller than that of official products.   
__So this module may not work normally when supplied from the on-board 3v3.__   

(*2)    
RFM69 is not 5V tolerant.   
You need level shift from 5V to 3.3V.   
I used [this](https://www.ti.com/lit/ds/symlink/txs0108e.pdf?ts=1647593549503) for a level shift.   

