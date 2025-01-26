# Example of Arduino environment
I used [this](https://github.com/adafruit/RadioHead) library.   

# Wireing   
|RFM69||ATmega328|ESP8266|
|:-:|:-:|:-:|:-:|
|MISO|--|D12(*2)|D12(*2)|
|SCK|--|D13|D14|
|MOSI|--|D11|D13|
|NSS|--|D4|DO2|
|DIO0|--|D3(*2)|D15(*2)|
|RESET|--|D2|D16|
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

# Configuration

- Radio frequency   
 Specifies the frequency to be used.
```
#define RF69_FREQ 915.0
```

- Tx power   
 If you are using a high power RF69 eg RFM69HW, you __must__ set a Tx power with the ishighpowermodule flag set like this:
```
rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW
```

