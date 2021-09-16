#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "it8951/ed078kc2.h"

// Constructor
Ed078Kc2::Ed078Kc2(EpdFastSpi& dio): 
  Adafruit_GFX(ED078KC2_WIDTH, ED078KC2_HEIGHT),
  It8951Base(ED078KC2_WIDTH, ED078KC2_HEIGHT, dio), IO(dio)
{
  printf("Ed078Kc2() %d*%d\n",
  ED078KC2_WIDTH, ED078KC2_HEIGHT);
}

// Destructor
Ed078Kc2::~Ed078Kc2() {
  IO.release();
}

//Initialize the display
void Ed078Kc2::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) {
      printf("Ed078Kc2::init(%d) bufferSize: %d width: %d height: %d\n", 
    debug, ED078KC2_BUFFER_SIZE, ED078KC2_WIDTH, ED078KC2_HEIGHT);
    }
    initIO(debug);
    
    _wakeUp();

}

void Ed078Kc2::clearScreen(uint8_t value){
  _initial_write = false; // initial full screen buffer clean done
  if (_initial_refresh) {
    _Init_Full();
  }  else {
    _Init_Part();
    _initial_refresh = false;
    _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  }
  IO.data16(0x0000); // preamble for write data
  _waitWhileBusy2("clearScreen preamble", default_wait_time);

  for (uint16_t x = 0; x < ED078KC2_BUFFER_SIZE; x++)
  {
    _buffer[x] = value;
    IO.data(value);
  }

  _writeCommand16(IT8951_TCON_LD_IMG_END);
  _waitWhileBusy2("clearScreen load end", default_wait_time);
  //_refresh(0, 0, WIDTH, HEIGHT, false);
}

void Ed078Kc2::update(bool partial_update_mode)
{
  ESP_LOGD(TAG, "Sending %d bytes buffer", ED078KC2_BUFFER_SIZE);
  //_waitBusy("Buffer sent", EPD_TMG_SRT);
  
  _powerOn();

  _powerOff();
}

void Ed078Kc2::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // Check rotation and move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = ED078KC2_WIDTH - x - 1;
      break;
    case 2:
      x = ED078KC2_WIDTH - x - 1;
      y = ED078KC2_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = ED078KC2_HEIGHT - y - 1;
      break;
  }
  
  // Check how to set this pixel on buffer on epd_draw_pixel
  uint8_t *buf_ptr = &_buffer[y * ED078KC2_WIDTH / 2 + x / 2];
  if (x % 2) {
    *buf_ptr = (*buf_ptr & 0x0F) | (color & 0xF0);
  } else {
    *buf_ptr = (*buf_ptr & 0xF0) | (color >> 4);
  }
}

void Ed078Kc2::_writeData16(uint16_t d)
{
  _waitBusy("_writeData16", default_wait_time);
  IO.data16(0x0000);
  _waitBusy("_writeData16 preamble", default_wait_time);
  IO.data16(d);
}

void Ed078Kc2::_writeCommand16(uint16_t c)
{
  String s = String("cmd(0x") + String(c, HEX) + String(")");
  _waitWhileBusy2(s.c_str(), default_wait_time);

  IO.data16(0x6000); // preamble for write command
  _waitBusy("_writeCommand16 preamble", default_wait_time);
  IO.data16(c);
}

void Ed078Kc2::_writeCommandData16(uint16_t c, const uint16_t* d, uint16_t n)
{
  _writeCommand16(c);
  for (uint16_t i = 0; i < n; i++)
  {
    IO.data16(d[i]);
  }
}

void Ed078Kc2::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t usArg[5];
  usArg[0] = (IT8951_LDIMG_B_ENDIAN << 8 ) | (IT8951_8BPP << 4) | (IT8951_ROTATE_0);
  usArg[1] = x;
  usArg[2] = y;
  usArg[3] = w;
  usArg[4] = h;
  _writeCommandData16(IT8951_TCON_LD_IMG_AREA , usArg , 5);
}