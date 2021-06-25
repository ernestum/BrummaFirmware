# Brumma Firmware

Brumma is a driver hardware to drive a chain of vibrating motors. This is the Firmware for an Arduino Nanoe 33 BLE connected to Brumma.

## Connecting via Serial
The Brumma Firmware provides a serial interface at 115200 baud for debugging purposes. It accepts the following commands (terminated with a newline):
Command | 
--------|----
`h` or `H` | display help text
`p<number>` | set the power of the motors between 0 and 100
`0010010` |or any variation thereof enables/disables the motors. The length has to match the number of motors

## Connecting via Bluetooth BLE
> TODO

## History
The firmware code is based on the [TingleDongleServer](https://github.com/ernestum/TingleDongleServer) which ran on an esp32 and allowed to connect to vibrating motors via a web server.