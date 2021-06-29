# stm32f103_ds18b20_modbus

_Welcome to the tm32f103_ds18b20_modbus project main repository!_ Data
acquisition device for digital thermometers ds18b20 and modbus-tcp server.

WARNING: This project is work in progress! Anything can (and probably
will) change! Do not use the code found in this project for any
critical system(s). The use of the code and documentation is at your
own risk!

In this project he uses a *Blue Pill* (stm32f103c8t6) and a *W5500*
ethernet board connected to the SPI1 interface. You can use a 16x2 LCD
display, a rotary encoder with switch and a bmp180 barometer. See
[eschematic.pdf](eschematic.pdf)

By default, up to 77 bs18b20 thermometers can be configured, which are
read every two seconds. The information of the thermometers is
available through the ModBus protocol. The code uses the HAL library
and was generated with STM32CubeMX initially.

A configuration interface is implemented by _telnet_ to port 500.

**You can choose to enable or disable the following features or
  modules selecting it in [my_config.h](Inc/my_config.h):**

*WDT*<br>
*RTC*<br> 
*NTP client (on W5500).*<br>
*DNS client (on W5500).*<br>
*LCD 16x02 (via SPI2 on 74HC595).*<br>
*Thermometers ds18b20 (via OneWire, driver implemented in hardware).*<br>
*Rotary encoder with switch (via timer 3).*<br>
*Barometer BMP180 (via I2C).*<br>
*ModBus server (TCP).*<br>
*Debugin.*<br>
*UART1 as a monitor.*<br>

