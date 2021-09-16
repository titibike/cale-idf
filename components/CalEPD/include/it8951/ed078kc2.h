#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    static const uint16_t reset_to_ready_time = 1800; // ms, e.g. 1729001us
    static const uint16_t power_on_time = 10;         // ms, e.g. 3001us
    static const uint16_t power_off_time = 250;       // ms, e.g. 214001us
    static const uint16_t full_refresh_time = 650;    // ms, e.g. 629001us
    static const uint16_t partial_refresh_time = 300; // ms, e.g. 287001us
    static const uint16_t refresh_cmd_time = 10;      // ms, e.g. 6052us
    static const uint16_t refresh_par_time = 2;       // ms, e.g. 1921us
    static const uint16_t default_wait_time = 1;      // ms, default busy check, needed?
    static const uint16_t diag_min_time = 3;          // ms, e.g. > refresh_par_time
    static const uint16_t set_vcom_time = 500;        // ms, e.g. 454967us

    void init(bool debug = false);
    void clearScreen(uint8_t value = 0x33);                  // init controller memory and screen (default white)
    void writeScreenBuffer(uint8_t value = 0x33);            // init controller memory (default white)

    void update(bool partial_update_mode = false);           // screen refresh from controller memory to full screen
    // Partial pending to be implemented
    //void update(int16_t x, int16_t y, int16_t w, int16_t h); // screen refresh from controller memory, partial screen
    void powerOff();  // turns off generation of panel driving voltages, avoids screen fading over time
    void hibernate(); // turns powerOff() and sets controller to deep sleep for minimum power use, ONLY if wakeable by RST (rst >= 0)
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
    // IT8951
    void _waitWhileBusy2(const char* comment = 0, uint16_t busy_time = 5000);
    uint16_t _transfer16(uint16_t value);
    void _writeCommand16(uint16_t c);
    void _writeData16(uint16_t d);
    void _writeData16(const uint16_t* d, uint32_t n);
    uint16_t _readData16();
    void _readData16(uint16_t* d, uint32_t n);
    void _writeCommandData16(uint16_t c, const uint16_t* d, uint16_t n);
    void _IT8951SystemRun();
    void _IT8951StandBy();
    void _IT8951Sleep();
    uint16_t _IT8951ReadReg(uint16_t usRegAddr);
    void _IT8951WriteReg(uint16_t usRegAddr, uint16_t usValue);
    uint16_t _IT8951GetVCOM(void);
    void _IT8951SetVCOM(uint16_t vcom);
};