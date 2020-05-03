# CALE ESP-IDF beta

This is the beginning, and a very raw try, to make CALE compile in the Espressif IOT Development Framework. At the moment to explore how difficult it can be to pass an existing ESP32 Arduino framework project to a ESP-IDF based one and to measure how much Framerate we can get compiling this with Espressif's own dev framework. 
It will take some weeks to have a working example. The reason is that we would like to explore alternative libraries like [ESP32 IDF Epaper example](https://github.com/loboris/ESP32_ePaper_example) and to make a version that is compatible with the recently released ESP32-S2

### Compile this 

If you have ESP32S2BETA (The ones that Espressif sent before official release)

    idf.py -D IDF_TARGET=esp32s2beta menuconfig

If it's an ESP32S2

    idf.py -D IDF_TARGET=esp32s2 menuconfig

And then just build and flash

    idf.py build
    idf.py flash

To open the serial monitor

    idf.py monitor

Please note that to exit the monitor in Espressif documentation says Ctrl+] but in Linux this key combination is:

    Ctrl+5
