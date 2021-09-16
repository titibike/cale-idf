#include <calepd_version.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <stdint.h>
#include <math.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include <string>
#include <Adafruit_GFX.h>
#include <it8951/epdfastspi.h>

#pragma once
#define EPD_WHITE 0xff
#define EPD_BLACK 0x00
#define VCOM  1500 //e.g. -1.53 = 1530 = 0x5FA

//Built in I80 Command Code
#define IT8951_TCON_SYS_RUN      0x0001
#define IT8951_TCON_STANDBY      0x0002
#define IT8951_TCON_SLEEP        0x0003
#define IT8951_TCON_REG_RD       0x0010
#define IT8951_TCON_REG_WR       0x0011
#define IT8951_TCON_LD_IMG       0x0020
#define IT8951_TCON_LD_IMG_AREA  0x0021
#define IT8951_TCON_LD_IMG_END   0x0022

//I80 User defined command code
#define USDEF_I80_CMD_DPY_AREA     0x0034
#define USDEF_I80_CMD_GET_DEV_INFO 0x0302
#define USDEF_I80_CMD_DPY_BUF_AREA 0x0037
#define USDEF_I80_CMD_VCOM       0x0039

//Rotate mode
#define IT8951_ROTATE_0     0
#define IT8951_ROTATE_90    1
#define IT8951_ROTATE_180   2
#define IT8951_ROTATE_270   3

//Pixel mode , BPP - Bit per Pixel
#define IT8951_2BPP   0
#define IT8951_3BPP   1
#define IT8951_4BPP   2
#define IT8951_8BPP   3

//Endian Type
#define IT8951_LDIMG_L_ENDIAN   0
#define IT8951_LDIMG_B_ENDIAN   1

#define SYS_REG_BASE 0x0000
#define I80CPCR (SYS_REG_BASE + 0x04)
#define MCSR_BASE_ADDR 0x0200
#define LISAR (MCSR_BASE_ADDR + 0x0008)

// Common methods for all IT8951 controllers
class It8951Base : public virtual Adafruit_GFX
{
  public:
    const char* TAG = "IT8951";
    
    It8951Base(int16_t w, int16_t h, EpdFastSpi& dio) : Adafruit_GFX(w,h), IO(dio) {
        printf("CalEPD component version %s. Class: It8951Base\n",CALEPD_VERSION);
    };

    void initIO(bool debug = false);

    // Every display model should implement this public methods
    // Override GFX own drawPixel method
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void init(bool debug = false) = 0;
    virtual void update(bool partial_update_mode = false) = 0;  // screen refresh from controller memory to full screen
    //virtual void update(int16_t x, int16_t y, int16_t w, int16_t h);

    // This are common methods every MODELX will inherit
    size_t write(uint8_t);  // hook to Adafruit_GFX::write
    void print(const std::string& text);
    void println(const std::string& text);
    void newline();
    void _powerOn();
    void _powerOff();

  // Methods that should be accesible by inheriting this abstract class
  protected: 
    bool debug_enabled = true;
    // Very smart template from EPD to swap x,y:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
    void _setSize(uint8_t epdSize);
    void _wakeUp();
    void _waitBusy(const char* message, uint16_t busy_time);

  private:
    // Only detail IO is being instanced two times and may be not convenient:
    EpdFastSpi& IO;
    
    uint8_t _unicodeEasy(uint8_t c);
};
