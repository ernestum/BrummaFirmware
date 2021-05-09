# TingleDongle Server

A webserver for the ESP32, that allows to control vibration motors attached to chained shift registers aka TingleDongles:

![TingleDongle Hardware](resources/Overview.png)

## Usage
 Step                                                         | QR Code                                                      
 ------------------------------------------------------------ | ------------------------------------------------------------ 
 Log into the TingleDongle Servers WLAN called `Tingledongle` using the password `tingletingle`. You can use the QR code. | ![Login with QR code](https://zxing.org/w/chart?cht=qr&chs=230x230&chld=L&choe=UTF-8&chl=WIFI%3AS%3ATingledongle%3BT%3AWPA%3BP%3Atingletingle%3B%3B) 
 You will find the GUI at http://tingledongle.local or alternatively at http://192.168.4.1. Here you can set the motor power and toggle individual motors as well as all motors together. | ![Web UI](https://zxing.org/w/chart?cht=qr&chs=350x350&chld=L&choe=UTF-8&chl=http%3A%2F%2F192.168.4.1) 
 You can make the ESP32 log into your home WLAN by clicking "Changle WLAN". This way you can access the internet and control the vibration motors at the same time. | ![Change WLAN](https://zxing.org/w/chart?cht=qr&chs=350x350&chld=L&choe=UTF-8&chl=http%3A%2F%2F192.168.4.1%2Fauth) 


## Pin Layout
SPI is used to pipe data to the shift registers.
PWN on a separate enable line controls the motor power for all motors together.

Function | Number
---------|-------:
Enable   | 17
MOSI     | 23
Clock  	 | 19
Chip Select | 16

