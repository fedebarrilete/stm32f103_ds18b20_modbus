# STM32f103 ds18b20 ModBus-TCP

_Welcome to the stm32f103_ds18b20_modbus project main repository!_ Data
acquisition device for digital thermometers ds18b20 and modbus-tcp server.

WARNING: This project is work in progress! Anything can (and probably
will) change! Do not use the code found in this project for any
critical system(s). The use of the code and documentation is at your
own risk!

In this project he uses a *Blue Pill* (stm32f103c8t6) and a
[W5500](http://wizwiki.net/wiki/doku.php?id=products:w5500:start)
ethernet board connected to the SPI1 interface. You can use a 16x2 LCD
display, a rotary encoder with switch and a _BMP180_ barometer. See
[eschematic.pdf](eschematic.pdf)

By default, up to 77 bs18b20 thermometers can be configured, which are
read every two seconds. The information of the thermometers is
available through the ModBus protocol. The code uses the HAL library
and was generated with STM32CubeMX initially.

A configuration interface is implemented by _telnet_ to port 500.

**You can choose to enable or disable the following features or
  modules selecting it in `Inc/my_config.h`:**

 - WDT
 - RTC
 - NTP client (on W5500)
 - DNS client (on W5500)
 - LCD 16x02 (via SPI2 on 74HC595)
 - Thermometers ds18b20 (via OneWire, driver implemented in hardware)
 - Rotary encoder with switch (via timer 3)
 - Barometer BMP180 (via I2C)
 - Hygrometer AM2302/DHT22 (via propietary onewire protocol)
 - ModBus server (TCP)
 - Debugin
 - UART1 as a monitor


## Quick Star:

You only need the _Blue Pill_ and _W5500_ module and connect them as follows:

Blue Pill | W5500 Board
----------|------
PA5 SPI1_SCK  | 10 SCLC
PA6 SPI1_MISO |  7 MISO
PA7 SPI1_MOSI |  8 MOSI
PB0 W5500_CS  |  9 SCS
PB9 W5500_RST |  3 RST
GNG           |  4 GNG
VCC 5v        |  5 5v

To start using only the ethernet module, you can configure the flags
as follows in `Inc/my_config.h`:

    #define MY_CFG_UPRINTF_ENABLE 1L
    #define MY_CFG_DEBUG 0L
    #define MY_CFG_WDT_ENABLE       0L
    #define MY_CFG_NTP_ENABLE       1L 
    #define MY_CFG_RTC_ENABLE       0L /* FIXME: make long test in disable state. */
    #define MY_CFG_DNS_ENABLE       0L /* It is not convenient to enable it. */
    #define MY_CFG_LCD_ENABLE       0L 
    #define MY_CFG_18B20_ENABLE     0L 
    #define MY_CFG_ENCODER_ENABLE   0L /* Specify the type of encoder in
    				    * encoder_sw.h */
    #define MY_CFG_BAROMETER_ENABLE 0L /* BMP 180 */
    #define MY_CFG_DHT_ENABLE       0L /* DHT (Humidity-Temperature) sensors */
    #define MY_CFG_MODBUS_ENABLE    1L
    #define MY_CFG_MENUCFG_ENABLE   1L /* size ~ 6.5K */ 

If you decide to enable the RTC, you need to power the VBAT pin. 

If you are going to record with st-link, just type:

    make
    make flash

If you don't have st-link, you can modify `local.makefile` and record with a USB-TTL converter:

    flash: all
        stm32flash -w build/${TARGET}.bin -g 0x0 /dev/ttyUSB0
        #st-flash write build/${TARGET}.bin 0x8000000

When the program starts, the W5500 board needs about 9 seconds to
work. With a TTL-USB module you can monitor the Tx pin (PA9) of the
UART1 and obtain the assigned IP number.

Now you can use the configuration interface via _telnet_:

    telnet YOUR.IP.NUMBER 500

The modbus protocol does not depend on specific hardward so it can be
enabled from the beginning. Use the tool you want to communicate by
modbus-tcp to port 502. In the file [Inc/mb_addrs.h](Inc/mb_addrs.h)
the addresses of the protocol are described.

An excellent tool for testing and debugging with modbus-tcp protocol
is [mbpoll](https://github.com/epsilonrt/mbpoll).

