/**
 * @file eco_depg02.h
 * @author your name (you@domain.com)
 * @brief Driver for depg0290BNS800F6 
 * @version 0.1
 * @date 2022-07-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
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
#include <epd.h>
#include <Adafruit_GFX.h>
#include <epdspi.h>
#include "soc/rtc_wdt.h"
#include <gdew_colors.h>


// 2.66"
// _widthScreen = 296; // x = wide size
// _heightScreen = 152; // y = small size
// _screenDiagonal = 266;
// _refreshTime = 15;

#define ECO_DEPG02_WIDTH 128
#define ECO_DEPG02_HEIGHT 296

// EPD comment: Pixel number expressed in bytes; this is neither the buffer size nor the size of the buffer in the controller
// We are not adding page support so here this is our Buffer size
#define ECO_DEPG02_BUFFER_SIZE (uint32_t(ECO_DEPG02_WIDTH) * uint32_t(ECO_DEPG02_HEIGHT) / 8)

// 8 pix of this color in a buffer byte:
#define ECO_DEPG02_8PIX_BLACK 0x00
#define ECO_DEPG02_8PIX_WHITE 0xFF

struct LUT_data
{
	uint8_t panelSet[2];
	uint8_t vcomIntrval[1];
	uint8_t vcomDC[1];
	uint8_t PLLframert[1];
	uint8_t lutC[42];
	uint8_t lutWb_W[42];
	uint8_t lutBW_R[42];
	uint8_t lutWW[42];
	uint8_t lutBB_B[42];
	uint8_t vcomIntrval_fix[1];
};


class EcoDEPG02 : public Epd
{
  public:
   
    EcoDEPG02(EpdSpi& IO);
    EpdSpi& IO;
    
    uint8_t colors_supported = 3;
    LUT_data ltb;
    uint8_t _buffer[ECO_DEPG02_BUFFER_SIZE];
    uint8_t _previous_buffer[ECO_DEPG02_BUFFER_SIZE];

    void drawPixel(int16_t x, int16_t y, uint16_t color);  // Override GFX own drawPixel method
    
    // EPD tests 
    void init(bool debug = false);
    // Partial update of rectangle from buffer to screen, does not power off
    void updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void fillScreen(uint16_t color);
    void update();
    void _Fullreset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5);
    void initFastUpdate(LUT_data ltc);
    void initPartialUpdate(uint8_t partialImgConfig[5]);
    void fastUpdateTest(const unsigned char* fastImgSet[], uint8_t fastImgSize,uint8_t numLoops);
    void partialUpdateTest(const unsigned char* partialImgSet[], uint8_t partialImgConfig[5], long windowSize, uint8_t numLoops);
    void updateLUT(LUT_data *ltc);
    void globalUpdate(const uint8_t * data1s, const uint8_t * data2s);
    void fastUpdate();
    void fastUpdateInit();
    void printBuffer();
    void _clearWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h);
    void addSpiDevice();
  private:
    

    
    bool _using_partial_mode = false;
    bool _initial = true;
    
    void _wakeUp();
    void _sleep();
    void _waitBusy(const char* message);
    void _rotate(uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h);
    uint16_t _setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
    void _partialRamArea(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void _writeToWindow(uint8_t command, uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h);
    void _refreshWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
 
    // Partial LUT tables (Otherwise partial update lasts long)
    static const epd_init_44 lut_20_vcomDC_partial;
    static const epd_init_42 lut_21_ww_partial;
    static const epd_init_42 lut_22_bw_partial;
    static const epd_init_42 lut_23_wb_partial;
    static const epd_init_42 lut_24_bb_partial;
    
};
