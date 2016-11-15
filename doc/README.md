## Intro

This project uses the [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk) SDK, with a custom lcd driver and custom [backend](no-url-yet) to draw images as fast as possible to an ILI9431 tft display.


I wanted to optimize [FastBMP](https://github.com/agashka/FastBMP) even further. Starting off with the [Blinky](https://github.com/esp8266/source-code-examples/tree/master/blinky) example & awesome Makefile, I created this project in order to avoid the Arduino libraries overhead.



## The proccess and what I've learned

The lack of official documentation on the chip is quite a bummer. Because of that, I used a logic analyzer from the start. Replicating the initialization routine and getting the LCD to fill one color took me roughly one night. While the initilization commands / data values were copy/pasted from Adafruits library (no shame, the lcd datasheet doesn't really cover in depth the init wizardery), the rest of the code was developped independently while looking at the output from the Saleae Logic software.

My WeMos D1 Mini also gave me issues with esptool.py, but thankfully [using a different baud rate](https://www.reddit.com/r/esp8266/comments/4ga2tv/wemos_d1_mini_connection_issues_on_osx_1011/) seemed to solve the issue for me.


![alt text](doc/adafruit.png "Adafruit startup routine")
Adafruits original startup routine, as captured by Saleae Logic. Surprisingly, a lot of time spent in-between SPI transactions!

![alt text](doc/optimized.png "Optimized startup routine")
Somewhat optimized routine, ommitting the CS signal as shown optional by the datasheet. Also showing 24-bit SPI transactions.


![alt text](doc/ili9341.png "ILI9341 Serial protocol")
Since SDA/SLC are handled by the SPI library (CS too, turns out!), we only need to make sure DC is LOW during commands, and HIGH during data transfers. Optionally CS can be kept LOW during larger transfers.



## TL;DR: Optimizations

- Very low overhead: uses RTOS and [HSPI driver](https://github.com/MetalPhreak/ESP8266_SPI_Driver), everything else is custom.
- Display initilization routine rewritten to be faster, more compact.
- Uses all 16 SPI registers for 64-bytes transactions when needed.
- [Faster SPI](http://www.eevblog.com/forum/microcontrollers/ili9341-lcd-driver-max-spi-clock-speed/) communication with the display.
- Custom image format (RGB565 really) means no data conversion.



## What else could be improved

- Look into DMA possibilities. [doable in UART](http://bbs.espressif.com/viewtopic.php?f=7&t=48&p=173#p173)
- Falling / Rising edge triggers, SPI Mode (single/double/quad?)
- Assembly, although I'm [not interested](http://bbs.espressif.com/viewtopic.php?t=407)
- Switching to software SPI that runs at optimal frequency.



## References & Documentation

- ["Faster GPIOs on ESP8266"](https://github.com/wdim0/esp8266_direct_gpio) by WDIM0
- ["ESP8266 Output GPIO and Optimization"](http://www.openmyr.com/blog/2016/06/esp8266-output-gpio-and-optimization/) by OpenMYR
- ["HSPI Clock Configuration Registers Explained"](http://www.esp8266.com/viewtopic.php?p=13958) by MetalPhreak
- ["ESP8266 native SPI hardware driver"](http://www.eevblog.com/forum/microcontrollers/esp8266-native-spi-hardware-driver/) (SPI registers observations) by MetalPhreak
- [ILI9341 datasheet](https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf) by Adafruit
- [ESP8266 Technical Reference](https://espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf) and [ESP8266 Specifications](https://cdn-shop.adafruit.com/datasheets/ESP8266_Specifications_English.pdf) by Espressif