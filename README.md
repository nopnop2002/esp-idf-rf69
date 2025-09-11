# esp-idf-rf69
RFM69 ISM Transceiver driver for esp-idf.   
The RFM69 is a highly integrated RF transceiver capable of operation over a wide frequency range, including the 433, 868 and 915 MHz license-free ISM (Industry Scientific and Medical) frequency bands.   
I ported from [this](http://www.airspayce.com/mikem/arduino/RadioHead/).

I tested with these.   
RFM69W/RFM69CW has +13 dBm Power Output Capability.   
RFM69HW/RFM69HCW has +20 dBm Power Output Capability.   
![RFM69-3](https://user-images.githubusercontent.com/6020549/168982514-439e93a1-5633-4cf2-9c99-8490e38107f5.JPG)
![RFM69-12](https://user-images.githubusercontent.com/6020549/168982527-f090f229-dfec-4473-8e0b-9a5d4d77d742.JPG)

# Foot pattern
RFM69CW/HCW has the same foot pattern as ESP12.   
Therefore, a pitch conversion PCB for ESP12 can be used.   
![RFM69CW](https://user-images.githubusercontent.com/6020549/168983702-3d0e8cac-add8-4906-bbfe-22eeee576ff7.JPG)
![RFM69HCW](https://user-images.githubusercontent.com/6020549/168983707-4bb3170a-47ae-4225-b87a-3fc5cc2c07ab.JPG)

RFM69W/HW does not have the same foot pattern as ESP12.
![RFM69HW](https://user-images.githubusercontent.com/6020549/168983973-73f21359-21f3-4833-a7ff-dc329faa504f.JPG)
![RFM69W](https://user-images.githubusercontent.com/6020549/168983977-bedada69-5722-46fb-839e-37ec77cc2b26.JPG)


# Software requirements
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   
ESP-IDF V5.1 is required when using ESP32-C6.   

# Installation

```Shell
git clone https://github.com/nopnop2002/esp-idf-rf69
cd esp-idf-rf69/basic
idf.py set-target {esp32/esp32s2/esp32s3/esp32c2/esp32c3/esp32c6}
idf.py menuconfig
idf.py flash
```

__Note for ESP32C3__   
For some reason, there are development boards that cannot use GPIO06, GPIO08, GPIO09, GPIO19 for SPI clock pins.   
According to the ESP32C3 specifications, these pins can also be used as SPI clocks.   
I used a raw ESP-C3-13 to verify that these pins could be used as SPI clocks.   


# Configuration for Transceiver   
![config-rf69-1](https://user-images.githubusercontent.com/6020549/168982654-f570bf49-1e23-4c82-a477-bb6cb9efb685.jpg)
![config-rf69-2](https://user-images.githubusercontent.com/6020549/210901663-fbbc2f25-1c17-4192-b35b-c921aba8389d.jpg)

- Set TX power high   
 If you are using a high power RF69 such as the RFM69HW/RFM69HCW, you __must__ to set the TX power high.   
 If you are using a not high power RF69 such as the RFM6W/RFM69CW, you don't need to set the TX power high.   


# SPI BUS selection   
![config-rf69-3](https://user-images.githubusercontent.com/6020549/168986794-f253634a-d982-4103-a439-a26f5b822644.jpg)

The ESP32 series has three SPI BUSs.   
SPI1_HOST is used for communication with Flash memory.   
You can use SPI2_HOST and SPI3_HOST freely.   
When you use SDSPI(SD Card via SPI), SDSPI uses SPI2_HOST BUS.   
When using this module at the same time as SDSPI or other SPI device using SPI2_HOST, it needs to be changed to SPI3_HOST.   
When you don't use SDSPI, both SPI2_HOST and SPI3_HOST will work.   
Previously it was called HSPI_HOST / VSPI_HOST, but now it is called SPI2_HOST / SPI3_HOST.   

# Wiring

|RFM69||ESP32|ESP32-S2/S3|ESP32-C2/C3/C6|
|:-:|:-:|:-:|:-:|:-:|
|MISO|--|GPIO19|GPIO37|GPIO4|
|SCK|--|GPIO18|GPIO36|GPIO3|
|MOSI|--|GPIO23|GPIO35|GPIO2|
|NSS|--|GPIO5|GPIO34|GPIO1|
|RESET|--|GPIO16|GPIO38|GPIO0|
|GND|--|GND|GND|GND|
|VCC|--|3.3V|3.3V|3.3V|

__You can change it to any pin using menuconfig.__   

__The pinout of RFM69 is different for each model.__   


# API
http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF69.html   

Interrupts are not used in this project.

# How to use this component in your project   
Create idf_component.yml in the same directory as main.c.   
```
YourProject --+-- CMakeLists.txt
              +-- main --+-- main.c
                         +-- CMakeLists.txt
                         +-- idf_component.yml
```

Contents of idf_component.yml.
```
dependencies:
  nopnop2002/rf69:
    path: components/rf69/
    git: https://github.com/nopnop2002/esp-idf-rf69.git
```

When you build a projects esp-idf will automaticly fetch repository to managed_components dir and link with your code.   
```
YourProject --+-- CMakeLists.txt
              +-- main --+-- main.c
              |          +-- CMakeLists.txt
              |          +-- idf_component.yml
              +-- managed_components ----- nopnop2002__rf69
```

# Comparison of RF69 and cc1101
||RF69|cc1101|
|:-:|:-:|:-:|
|Manufacturer|Hope Microelectronics|Texas Instrument|
|Frequency|433/868/915MHz|315/433/868/915MHz|
|Maximum Payload|64Byte(AES Enable)/255Byte(AES Disable)|64Byte|
|CRC Length|16bits|16bits|
|Acknowledgement Payload|No|No|
|Available Modulation format|FSK/GFSK/MSK/GMSJ/OOK|2-FSK/4-FSK/GFSK/ASK/OOK/MSK|

AES is AES-128 encryption/decryption.   
This project uses AES-128 encryption/decryption option.  
