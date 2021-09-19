#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include <stdint.h>
#include <math.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include <string>
#include <Adafruit_GFX.h>
#include <it8951/epdfastspi.h>
#include <it8951/it8951base.h>
#include "soc/rtc_wdt.h" // Watch cat

// Controller: IT8951 via ESP32 SPI Info: https://github.com/martinberlin/cale-idf/wiki/Model-parallel-ED078KC2
#define ED078KC2_WIDTH 1872
#define ED078KC2_HEIGHT 1404
#define HAS_16_LEVELS_GRAY 1
#define ED078KC2_BUFFER_SIZE (uint32_t(ED078KC2_WIDTH) * uint32_t(ED078KC2_HEIGHT) / 2)

class Ed078Kc2 : public It8951Base
{
  public:
    Ed078Kc2(EpdFastSpi& IO);
    ~Ed078Kc2();

    // Attributes. This definitions come from GxEPD2
    static const uint16_t WIDTH = ED078KC2_WIDTH;
    static const uint16_t HEIGHT = ED078KC2_HEIGHT;
    const uint8_t colors_supported = 1;
    static const bool hasPartialUpdate = true;
    bool _hibernating = false;
    bool _using_partial_mode = false;

    void init(bool debug = false);
    void clearScreen(uint8_t value = 0x33);                  // init controller memory and screen (default white)
    void writeScreenBuffer(uint8_t value = 0x33);            // init controller memory (default white)

    void update(bool partial_update_mode = false);           // screen refresh from controller memory to full screen
    // Partial pending to be implemented
    //void update(int16_t x, int16_t y, int16_t w, int16_t h); // screen refresh from controller memory, partial screen
    void drawPixel(int16_t x, int16_t y, uint16_t color);    // Replacing Adafruit's own drawPixel
  
  private:
    EpdFastSpi& IO;
    // Allocate buffer directly in external RAM
    uint8_t* _buffer = (uint8_t*)heap_caps_malloc(ED078KC2_BUFFER_SIZE, MALLOC_CAP_SPIRAM);

    struct IT8951DevInfoStruct
    {
      uint16_t usPanelW;
      uint16_t usPanelH;
      uint16_t usImgBufAddrL;
      uint16_t usImgBufAddrH;
      uint16_t usFWVersion[8];   //16 Bytes String
      uint16_t usLUTVersion[8];   //16 Bytes String
    };
    IT8951DevInfoStruct IT8951DevInfo;

    bool _initial = true;
    bool _debug_buffer = false;
    bool _initial_write = false;
    bool _initial_refresh = true;
    void _Init_Full();
    void _Init_Part();
    void _setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void _refresh(int16_t x, int16_t y, int16_t w, int16_t h, bool partial_update_mode = false);
};