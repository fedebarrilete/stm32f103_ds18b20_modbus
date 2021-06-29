# stm32f103_ds18b20_modbus
Data acquisition device for digital thermometers ds18b20 and modbus server.

WARNING: This project is work in progress! Anything can (and probably will) change! Do not use the code found in this project for any critical system(s). The use of the code and documentation is at your own risk!

In this project he uses a bluepill (stm32f103c8t6) and a W5500 ethernet board connected to the SPI1 interface. You can use a 16x2 LCD display, a rotary encoder with switch and a bmp180 barometer. See eschematic.pdf

By default, up to 77 bs18b20 thermometers can be configured, which are read every two seconds. The information of the thermometers is available through the ModBus protocol. 

A configuration interface is implemented by telnet to port 500. 
