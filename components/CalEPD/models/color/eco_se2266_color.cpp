#include "eco_se2266_color.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Controller: Aurora MB EC2266
// Specification: https://www.pervasivedisplays.com/wp-content/uploads/2019/06/ApplicationNote_Small_Size_Mono_v01_181022-6.pdf

// 0x07 (2nd) VGH=20V,VGL=-20V
// 0x3f (1st) VDH= 15V
// 0x3f (2nd) VDH=-15V

// This class is refactored to cope with Good display Arduino example
DRAM_ATTR const epd_init_3 EcoSE2266::epd_soft_start={
0x06,{0x07,0x07,0x17},3
};

DRAM_ATTR const epd_init_1 EcoSE2266::epd_extra_setting={
0x16,{0x00},1
};
// LUT from OTP 128x296
DRAM_ATTR const epd_init_1 EcoSE2266::epd_panel_setting={
0x00,{0x1f},1
};

DRAM_ATTR const epd_init_1 EcoSE2266::epd_vcom2={
0x50,{0x97},1
};

//partial screen update LUT
DRAM_ATTR const epd_init_44 EcoSE2266::lut_20_vcomDC_partial={
0x20, {
  0x00, 0x00,
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},44};

DRAM_ATTR const epd_init_42 EcoSE2266::lut_21_ww_partial={
0x21, {
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

DRAM_ATTR const epd_init_42 EcoSE2266::lut_22_bw_partial={
0x22, {
  0x80, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

DRAM_ATTR const epd_init_42 EcoSE2266::lut_23_wb_partial={
0x23, {
  0x40, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};

DRAM_ATTR const epd_init_42 EcoSE2266::lut_24_bb_partial={
0x24, {
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},42};
// Constructor
EcoSE2266::EcoSE2266(EpdSpi &dio) : Adafruit_GFX(EcoSE2266_WIDTH, EcoSE2266_HEIGHT),
                                    Epd(EcoSE2266_WIDTH, EcoSE2266_HEIGHT), IO(dio)
{
  printf("EcoSE2266() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         EcoSE2266_WIDTH, EcoSE2266_HEIGHT, EcoSE2266_BUFFER_SIZE);
  printf("\nAvailable heap after Epd bootstrap:%d\n", xPortGetFreeHeapSize());
}

//Initialize the display
void EcoSE2266::init(bool debug)
{
  debug_enabled = debug;
  if (debug_enabled)
    printf("EcoSE2266::init(debug:%d)\n", debug);
  //Initialize SPI at 4MHz frequency. true for debug
  IO.init(4, debug);
  fillScreen(EPD_WHITE);
}

void EcoSE2266::fillScreen(uint16_t color)
{
  uint8_t black = 0x00;
  uint8_t red = 0x00;
  if (color == EPD_WHITE);
  else if (color == EPD_BLACK) black = 0xFF;
  else if (color == EPD_RED) red = 0xFF;
  else if ((color & 0xF100) > (0xF100 / 2))  red = 0xFF;
  else if ((((color & 0xF100) >> 11) + ((color & 0x07E0) >> 5) + (color & 0x001F)) < 3 * 255 / 2) black = 0xFF;
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = black;
    _color[x] = red;
  }
}

void EcoSE2266::_Fullreset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5){
  IO.reset(ms1);// delay_ms 5ms
  IO.reset(ms2);// delay_ms 5ms
  IO.reset(ms3);// delay_ms 5ms
  IO.reset(ms4);// delay_ms 5ms
}



void EcoSE2266::_wakeUp()
{
  uint8_t data[]={0};
  vTaskDelay(5/portTICK_RATE_MS); // delay_ms 5ms
  gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
  vTaskDelay(5/portTICK_RATE_MS); // delay_ms 5ms
  gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 0);
  vTaskDelay(10/portTICK_RATE_MS); // delay_ms 5ms
  gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 1);
  vTaskDelay(5/portTICK_RATE_MS); // delay_ms 5ms

  gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS,1);
  
  IO.cmd(0x00); 
  data[0]=0x0e;
  IO.data(data,1); // Soft-reset
  vTaskDelay(5/portTICK_RATE_MS);

  IO.cmd(0xe5);
  data[0]=0x16;
  IO.data(data,1);// Input Temperature 0°C = 0x00, 22°C = 0x16, 25°C = 0x19

  
  IO.cmd(0xe0);// Active Temperature
  data[0]=0x02;
  IO.data(data,1);
  
  uint8_t data0[] = {0xcf, 0x8d};
  IO.cmd(0x00);
  IO.data(data0,2); // PSR
 
  uint8_t data4[] = {0x07};
  IO.cmd(0x50);
  IO.data(data4,1);// Vcom and data interval setting
  //_sendIndexData(0x50, data4, 1); 

  #if 1
  IO.cmd(0x04);
  data[0]=0x0;
  IO.data(data,1);
  _waitBusy("_wakeUp power on");
  #endif
  

  #if 0
  // Panel setting
  IO.cmd(0x00);
  IO.data(0x0f); //KW: 3f, KWR: 2F, BWROTP: 0f, BWOTP: 1f

  // PLL 
  IO.cmd(0x30);
  IO.data(0x06);

  // Resolution setting
  IO.cmd(epd_resolution.cmd);
  for (int i = 0; i < epd_resolution.databytes; ++i)
  {
    IO.data(epd_resolution.data[i]);
  }

  printf("Boost\n"); // Handles the intensity of the colors displayed
  IO.cmd(epd_boost.cmd);
  for (int i=0;i<sizeof(epd_boost.data);++i) {
    IO.data(epd_boost.data[i]);
  }

  // Not sure if 0x15 is really needed, seems to work the same without it too
  IO.cmd(0x15);  // Dual SPI
  IO.data(0x00); // MM_EN, DUSPI_EN

  IO.cmd(0x50);  // VCOM AND DATA INTERVAL SETTING
  IO.data(0x11); // LUTKW, N2OCP: copy new to old
  IO.data(0x07);

  IO.cmd(0x60);  // TCON SETTING
  IO.data(0x22);
  #endif
}

void EcoSE2266::update()
{
  uint64_t startTime = esp_timer_get_time();
  uint8_t dataCmd[]={0x0};
 // _using_partial_mode = false;
  int _sizePageColour=EcoSE2266_HEIGHT*EcoSE2266_WIDTH;

  
  //memcpy(_color,redBuffer,EcoSE2266_BUFFER_SIZE);
  uint8_t * nextBuffer = _buffer;
  uint8_t * previousBuffer = _buffer + _sizePageColour;
  if (!_using_partial_mode){
  _wakeUp();
  _using_partial_mode = false;
  }
  
  /* 
  IO.cmd(0x10);
  printf("Sending a %d bytes buffer via SPI\n", sizeof(_buffer));
  */
   // v2 SPI optimizing. Check: https://github.com/martinberlin/cale-idf/wiki/About-SPI-optimization
  
  #if 0
  uint16_t i = 0;
  uint8_t xLineBytes = EcoSE2266_WIDTH / 8;
  uint8_t x1buf[xLineBytes];
  for (uint16_t y = 1; y <= EcoSE2266_HEIGHT; y++)
  {
    for (uint16_t x = 1; x <= xLineBytes; x++)
    {
      uint8_t data = i < sizeof(_buffer) ? ~_buffer[i] : 0x00;
      x1buf[x - 1] = data;
      if (x == xLineBytes)
      { // Flush the X line buffer to SPI
        IO.data(x1buf, sizeof(x1buf));
      }
      ++i;
    }
  }

  IO.cmd(0x13);
  i = 0;
  for (uint16_t y = 1; y <= EcoSE2266_HEIGHT; y++)
  {
    for (uint16_t x = 1; x <= xLineBytes; x++)
    {
      uint8_t data = i < sizeof(_color) ? _color[i] : 0x00;
      x1buf[x - 1] = data;
      if (x == xLineBytes)
      { // Flush the X line buffer to SPI
        IO.data(x1buf, sizeof(x1buf));
      }
      ++i;
    }
  }
  vTaskDelay(50/portTICK_RATE_MS);

  #else
  printf("Hey bro \n\n");
  /* TEST OTHER METOD SPI*/
  IO.cmd(0x10);        // update old data
  IO.data(previousBuffer,EcoSE2266_BUFFER_SIZE);
  //IO.data(_buffer,EcoSE2266_BUFFER_SIZE); //TEST
  

  #if 1
  IO.cmd(0x13);        // update current data
  //IO.data(_color,EcoSE2266_BUFFER_SIZE);
  IO.data(nextBuffer,EcoSE2266_BUFFER_SIZE);
  memcpy(previousBuffer, nextBuffer, EcoSE2266_BUFFER_SIZE); // Copy displayed next to previous
  // IO.data(_buffer,sizeof(_buffer)); //TEST
   #endif

#endif
  uint64_t endTime = esp_timer_get_time();

  /* power on  */
  #if 1
  IO.cmd(0x04);
  dataCmd[0]=0;
  IO.data(dataCmd,1);
  _waitBusy("power on");
#endif

  #if 1
  IO.cmd(0x12);
  IO.data(dataCmd,1);
  vTaskDelay(50/portTICK_RATE_MS);
  _waitBusy("refresh");
  #endif

#if 0
IO.cmd(0x02);
#endif



  uint64_t updateTime = esp_timer_get_time();

  printf("\n\nSTATS (ms)\n%llu _wakeUp settings+send Buffer\n%llu update \n%llu total time in millis\n",
         (endTime - startTime) / 1000, (updateTime - endTime) / 1000, (updateTime - startTime) / 1000);
#if 1
gpio_set_level((gpio_num_t)CONFIG_EINK_SPI_CS, 0);
#endif

 //_sleep();
}

void EcoSE2266::_waitBusy(const char *message)
{
  if (debug_enabled)
  {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1)
  {
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1)
      break;
    vTaskDelay(1);
    if (esp_timer_get_time() - time_since_boot > 2000000)
    {
      if (debug_enabled)
        ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

void EcoSE2266::_sleep()
{
  uint8_t data[]={0x0};
  IO.cmd(0x02);
  IO.data(data,1);
  _waitBusy("power_off");
  
  gpio_set_level((gpio_num_t)CONFIG_EINK_DC, 0);
  gpio_set_level((gpio_num_t)CONFIG_EINK_RST, 0);
  
} 

void EcoSE2266::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    swap(w, h);
    x = EcoSE2266_WIDTH - x - w - 1;
    break;
  case 2:
    x = EcoSE2266_WIDTH - x - w - 1;
    y = EcoSE2266_HEIGHT - y - h - 1;
    break;
  case 3:
    swap(x, y);
    swap(w, h);
    y = EcoSE2266_HEIGHT - y - h - 1;
    break;
  }
}


void EcoSE2266::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
  switch (getRotation())
  {
  case 1:
    swap(x, y);
    x = EcoSE2266_WIDTH - x - 1;
    break;
  case 2:
    x = EcoSE2266_WIDTH - x - 1;
    y = EcoSE2266_HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = EcoSE2266_HEIGHT - y - 1;
    break;
  }
  uint16_t i = x / 8 + y * EcoSE2266_WIDTH / 8;

  if (color) {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    } else {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    }
}


void EcoSE2266::testbuff(int a){
  if (a==0){
  memcpy(_buffer,blackBuffer,EcoSE2266_BUFFER_SIZE);
  }else{
    memcpy(_buffer,redBuffer,EcoSE2266_BUFFER_SIZE);
  } 
}

void EcoSE2266::initPartialUpdate(){
    IO.cmd(0x00);  //Panel setting for fast partial
    IO.data(0xbf);

    IO.cmd(0x82);  //vcom_DC setting
    IO.data(0x08);

    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING
    IO.data(0x17); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

    uint8_t index = 0;
    IO.cmd(lut_20_vcomDC_partial.cmd);
    for (index = 0; index < lut_20_vcomDC_partial.databytes; index++) {
      IO.data(lut_20_vcomDC_partial.data[index]);
    }
   
    IO.cmd(lut_21_ww_partial.cmd);
    for (index = 0; index < lut_21_ww_partial.databytes; index++) {
      IO.data(lut_21_ww_partial.data[index]);
    }

    IO.cmd(lut_22_bw_partial.cmd);
    for (index = 0; index < lut_22_bw_partial.databytes; index++) {
      IO.data(lut_22_bw_partial.data[index]);
    }

    IO.cmd(lut_23_wb_partial.cmd);
    for (index = 0; index < lut_23_wb_partial.databytes; index++) {
      IO.data(lut_23_wb_partial.data[index]);
    }

    IO.cmd(lut_24_bb_partial.cmd);
    for (index = 0; index < lut_24_bb_partial.databytes; index++) {
      IO.data(lut_24_bb_partial.data[index]);
    }
    
    if (debug_enabled) printf("initPartialUpdate() LUT\n");
}

uint16_t EcoSE2266::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye) {
  IO.data(x >> 8);
  IO.data(x & 0xf8);
  IO.data(y >> 8);
  IO.data(y & 0xff);
  IO.data(xe >> 8);
  IO.data(xe & 0xf8);
  IO.data(ye >> 8);
  IO.data(ye & 0xff);
 return 1;
}

void EcoSE2266::_partialRamArea(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  IO.cmd(command);
  _setPartialRamArea(x,y,w,h);
}

void EcoSE2266::_refreshWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  w += (x % 8) + 7;
  h = gx_uint16_min(h, 256); // strange controller error
  IO.cmd(0x16);
  IO.data(x >> 8);
  IO.data(x & 0xf8);
  IO.data(y >> 8);
  IO.data(y & 0xff);
  IO.data(w >> 8);
  IO.data(w & 0xf8);
  IO.data(h >> 8);
  IO.data(h & 0xff);
}

void EcoSE2266::_writeToWindow(uint8_t command, uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
  //Serial.printf("_writeToWindow(%d, %d, %d, %d, %d, %d)\n", xs, ys, xd, yd, w, h);
  // the screen limits are the hard limits
  if (xs >= EcoSE2266_WIDTH) return;
  if (ys >= EcoSE2266_HEIGHT) return;
  if (xd >= EcoSE2266_WIDTH) return;
  if (yd >= EcoSE2266_HEIGHT) return;
  w = gx_uint16_min(w + 7, EcoSE2266_WIDTH - xd) + (xd % 8);
  h = gx_uint16_min(h, EcoSE2266_HEIGHT - yd);
  uint16_t xe = (xs / 8) + (w / 8);
  IO.cmd(0x91); // partial in
  _partialRamArea(command, xd, yd, w, h);
  for (uint16_t y1 = ys; y1 < ys + h; y1++)
  {
    for (uint16_t x1 = xs / 8; x1 < xe; x1++)
    {
      uint16_t idx = y1 * (EcoSE2266_WIDTH / 8) + x1;
      uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
      IO.data(~data);
    }
  }
  vTaskDelay(pdMS_TO_TICKS(2));
}

void EcoSE2266::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
  if (using_rotation) _rotate(x, y, w, h);
  // Only if sleep state is true:
  //if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  initPartialUpdate();
  _writeToWindow(0x15, x, y, x, y, w, h);
  _refreshWindow(x, y, w, h);   
  _waitBusy("updateWindow");
  _writeToWindow(0x14, x, y, x, y, w, h);
}