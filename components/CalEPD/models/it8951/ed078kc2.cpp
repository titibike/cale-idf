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
  printf("Ed078Kc2() %d*%d\n", ED078KC2_WIDTH, ED078KC2_HEIGHT);
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
     // Start SPI after long reset pulse
    initIO(debug);

    _waitBusy("reset_to_ready", reset_to_ready_time);
    // Todo: Get dev info (Width / Height and LUT version)
    //_writeCommand16(USDEF_I80_CMD_GET_DEV_INFO);
    //_waitBusy("GetIT8951SystemInfo", power_on_time);
    //Set to Enable I80 Packed mode
    _IT8951WriteReg(I80CPCR, 0x0001);

    _IT8951SetVCOM(VCOM);
    printf("Ed078Kc2::init() DONE\n");
}

void Ed078Kc2::_Init_Full()
{
  _InitDisplay();
  _powerOn();
  _using_partial_mode = false;
}

void Ed078Kc2::_Init_Part()
{
  _InitDisplay();
  _powerOn();
  _using_partial_mode = true;
}

void Ed078Kc2::clearScreen(uint8_t value){
  _initial_write = false;
  if (_initial_refresh) {
    _Init_Full();
  }  else {
    _Init_Part();
  }
  //IO.data16(0x0000); // preamble for write data
  uint8_t t16[2] = {0x00, 0x00};
  IO.data(t16, sizeof(t16));
  _waitBusy("clearScreen preamble", default_wait_time);

  _initial_refresh = false;

  //memset(_buffer, value, ED078KC2_BUFFER_SIZE);
  //Setting Load image information
  IT8951LdImgInfo stLdImgInfo;
  stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
  stLdImgInfo.usPixelFormat    = IT8951_4BPP; 
  stLdImgInfo.usRotate         = IT8951_ROTATE_0;
  loadImgStart(&stLdImgInfo);

  uint8_t xRow[ED078KC2_WIDTH/2];
  int32_t idx = 0;
  for (uint16_t y = 0; y < ED078KC2_HEIGHT; y++)
  {
    for (uint16_t x = 0; x < ED078KC2_WIDTH/2; x++)
    {
      xRow[x] = value;
      ++idx;
    }
    IO.data(xRow, sizeof(xRow));
    if (y%8==0) {
      vTaskDelay(12/ portTICK_RATE_MS);
      rtc_wdt_feed();
    }
  }
  printf("buffer size: %d\n", idx);

  _writeCommand16(IT8951_TCON_LD_IMG_END);
  _waitBusy("clearScreen load end", default_wait_time);
  _refresh(0, 0, WIDTH, HEIGHT, false);
}

void Ed078Kc2::_refresh(int16_t x, int16_t y, int16_t w, int16_t h, bool partial_update_mode)
{
  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  w1 -= x1 - x;
  h1 -= y1 - y;

  //Send I80 Display Command (User defined command of IT8951)
  _writeCommand16(USDEF_I80_CMD_DPY_AREA); //0x0034
  _waitBusy("refresh cmd", refresh_cmd_time);
  //Write arguments
  _writeData16(x1);
  _waitBusy("refresh x", refresh_par_time);
  _writeData16(y1);
  _waitBusy("refresh y", refresh_par_time);
  _writeData16(w1);
  _waitBusy("refresh w", refresh_par_time);
  _writeData16(h1);
  _waitBusy("refresh h", refresh_par_time);
  _writeData16(partial_update_mode ? 1 : 2); // mode
  _waitBusy("refresh", full_refresh_time);
}

void Ed078Kc2::update(bool partial_update_mode)
{
  ESP_LOGI(TAG, "Sending %d bytes buffer. Update mode:%d", ED078KC2_BUFFER_SIZE, (uint8_t)partial_update_mode);
  _initial_write = false; // initial full screen buffer clean done
  if (!_using_partial_mode) {
    _Init_Part();
  } else {
    _Init_Full();
  }
  //Setting Load image information
  IT8951LdImgInfo stLdImgInfo;
  stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
  stLdImgInfo.usPixelFormat    = IT8951_4BPP;
  stLdImgInfo.usRotate         = IT8951_ROTATE_0;
  loadImgStart(&stLdImgInfo);

  uint8_t t16[4] = {0x00, 0x00, 0x00, 0x00};
  IO.data(t16, sizeof(t16));
  _waitBusy("update/() preamble", default_wait_time);

  uint8_t xRow[ED078KC2_WIDTH/2];
  uint32_t idx = 0;
  for (uint16_t y = 0; y < ED078KC2_HEIGHT; y++)
  {
    for (uint16_t x = 0; x < ED078KC2_WIDTH/2; x++)
    {
      xRow[x] = _buffer[idx];
      ++idx;
    }

    IO.data(xRow, sizeof(xRow));
    if (y%20 == 0) {
      vTaskDelay(8/ portTICK_RATE_MS);
      rtc_wdt_feed();
    }
  }
  printf("Done sending img buffer : %d|%d\n", idx, ED078KC2_BUFFER_SIZE);

  _writeCommand16(IT8951_TCON_LD_IMG_END);
  _waitBusy("update load end", default_wait_time);
  _refresh(0, 0, WIDTH, HEIGHT);
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

void Ed078Kc2::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t usArg[5];
  // IT8951_8BPP
  usArg[0] = (IT8951_LDIMG_B_ENDIAN << 8 ) | (IT8951_4BPP << 4) | (IT8951_ROTATE_0);
  usArg[1] = x;
  usArg[2] = y;
  usArg[3] = w;
  usArg[4] = h;
  _writeCommandData16(IT8951_TCON_LD_IMG_AREA , usArg , 5);
}