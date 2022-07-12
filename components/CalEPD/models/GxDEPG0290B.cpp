// class GxDEPG0290B
// Controller :

#include "GxDEPG0290B.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <math.h> 
#include "gpio_expander.h"

#define EPD_HOST SPI2_HOST
#define ESP32 1

// Partial Update Delay, may have an influence on degradation
#define GxDEPG0290B_PU_DELAY 200


const uint8_t GxDEPG0290B::LUTDefault_part[] = {
    0x32,
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00,
    0x00, 0x00, //0x22, 0x17, 0x41, 0xB0, 0x32,
    // 0x36,
};


GxDEPG0290B::GxDEPG0290B(EpdSpi &dio) :
    Adafruit_GFX(GxDEPG0290B_WIDTH, GxDEPG0290B_HEIGHT),
    Epd(GxDEPG0290B_WIDTH, GxDEPG0290B_HEIGHT), IO(dio),
    _current_page(-1), _using_partial_mode(false), _diag_enabled(false)
{
    ESP_LOGI(TAG,"GxDEPG0290B() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         GxDEPG0290B_WIDTH, GxDEPG0290B_HEIGHT, GxDEPG0290B_BUFFER_SIZE);
  ESP_LOGI(TAG,"\nAvailable heap after Epd bootstrap:%d\n", xPortGetFreeHeapSize());
}

void GxDEPG0290B::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

    // check rotation, move pixel around if necessary
    switch (getRotation()) {
    case 1:
        swap(x, y);
        x = GxDEPG0290B_WIDTH - x - 1;
        break;
    case 2:
        x = GxDEPG0290B_WIDTH - x - 1;
        y = GxDEPG0290B_HEIGHT - y - 1;
        break;
    case 3:
        swap(x, y);
        y = GxDEPG0290B_HEIGHT - y - 1;
        break;
    }
    uint16_t i = x / 8 + y * GxDEPG0290B_WIDTH / 8;
    if (_current_page < 1) {
        if (i >= sizeof(_buffer)) return;
    } else {
        y -= _current_page * GxDEPG0290B_PAGE_HEIGHT;
        if ((y < 0) || (y >= GxDEPG0290B_PAGE_HEIGHT)) return;
        i = x / 8 + y * GxDEPG0290B_WIDTH / 8;
    }

    if (!color)
        _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    else
        _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
}

void GxDEPG0290B::init(bool debug)
{
     debug_enabled = debug;
    if (debug_enabled){
        ESP_LOGI(TAG,"EcoSE2266::init(debug:%d)\n", debug);
    }


    gpio_bike_set_level(GPIO_CS_ECRAN,1);
    gpio_bike_set_level(GPIO_ECRAN_CD,1);
    gpio_bike_set_level(GPIO_ECRAN_RESET,1);
    #if 0
    esp_err_t ret;
    // MISO not used, only Master to Slave
    spi_bus_config_t buscfg={
        .mosi_io_num=GPIO_SPI_MOSI,
        .miso_io_num = GPIO_SPI_MISO,
        .sclk_io_num=GPIO_SPI_SCLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=4094
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(EPD_HOST, &buscfg, EPD_HOST);
    ESP_ERROR_CHECK(ret);
    #endif
    gpio_bike_set_level(GPIO_ECRAN_RESET,0);
    vTaskDelay(10/portTICK_RATE_MS);
    gpio_bike_set_level(GPIO_ECRAN_RESET,1);
    vTaskDelay(10/portTICK_RATE_MS);
    _waitBusy("");
    
    fillScreen(EPD_WHITE);
    _current_page = -1;
    _using_partial_mode = false;
}

void GxDEPG0290B::addSpiDevice(){
  gpio_bike_t gpio_cs_ecran=getGpioBike(GPIO_CS_ECRAN);
  // max_transfer_sz   4Kb is the defaut SPI transfer size if 0
    // debug: 50000  0.5 Mhz so we can sniff the SPI commands with a Slave
    uint16_t multiplier = 1000;
    uint16_t frequency=8;
  //Config Frequency and SS GPIO
    spi_device_interface_config_t devcfg={
        .mode=0,  //SPI mode 0
        .clock_speed_hz=frequency*multiplier*1000,  // DEBUG: 50000 - No debug usually 4 Mhz
        //.clock_speed_hz=4000000,  // DEBUG: 50000 - No debug usually 4 Mhz
        .input_delay_ns=0,
        .spics_io_num=gpio_cs_ecran.gpio_pin,
        .flags = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),
        .queue_size=5
    };
    // DISABLED Callbacks pre_cb/post_cb. SPI does not seem to behave the same
    // CS / DC GPIO states the usual way

  //Attach the EPD to the SPI bus
    esp_err_t ret=spi_bus_add_device(EPD_HOST, &devcfg, &IO.spi);
    ESP_ERROR_CHECK(ret);
    if (debug_enabled) {
      ESP_LOGI(TAG,"EpdSpi::init() Debug enabled. SPI master at frequency:%d  CS_ECRAB:%d DMA_CH: %d\n",
      frequency*multiplier*1000,gpio_cs_ecran.gpio_pin, EPD_HOST);
        } 
    else {
        ESP_LOGI(TAG,"EpdSPI started at frequency: %d000\n", frequency*multiplier);
    }
}


void GxDEPG0290B::fillScreen(uint16_t color)
{
    uint8_t data = (color == EPD_BLACK) ? 0xFF : 0x00;
    for (uint16_t x = 0; x < sizeof(_buffer); x++) {
        _buffer[x] = data;
    }
}

void GxDEPG0290B::update(void)
{
    if (_current_page != -1) return;
    _using_partial_mode = false;
    _Init_Full(0x03);
    _writeCommand(0x24);
    for (uint16_t y = 0; y < GxDEPG0290B_HEIGHT; y++) {
        for (uint16_t x = 0; x < GxDEPG0290B_WIDTH / 8; x++) {
            uint16_t idx = y * (GxDEPG0290B_WIDTH / 8) + x;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
            _writeData(~data);
        }
    }
    _Update_Full();
    _PowerOff();
}

#if 1

void  GxDEPG0290B::drawBitmapBM(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  uint16_t inverse_color = (color != EPD_WHITE) ? EPD_WHITE : EPD_BLACK;
  uint16_t fg_color = (mode & bm_invert) ? inverse_color : color;
  uint16_t bg_color = (mode & bm_invert) ? color : inverse_color;
  // taken from Adafruit_GFX.cpp, modified
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;
  if (mode & bm_transparent)
  {
    for (int16_t j = 0; j < h; j++)
    {
      for (int16_t i = 0; i < w; i++ )
      {
        if (i & 7) byte <<= 1;
        else
        {

          byte = bitmap[j * byteWidth + i / 8];

        }
        // transparent mode
        if (bool(mode & bm_invert) != bool(byte & 0x80))
        //if (!(byte & 0x80))
        {
          uint16_t xd = x + i;
          uint16_t yd = y + j;
          if (mode & bm_flip_x) xd = x + w - i;
          if (mode & bm_flip_y) yd = y + h - j;
          drawPixel(xd, yd, color);
        }
      }
    }
  }
  else
  {
    for (int16_t j = 0; j < h; j++)
    {
      for (int16_t i = 0; i < w; i++ )
      {
        if (i & 7) byte <<= 1;
        else
        {

          byte = bitmap[j * byteWidth + i / 8];

        }
        // keep using overwrite mode
        uint16_t pixelcolor = (byte & 0x80) ? fg_color  : bg_color;
        uint16_t xd = x + i;
        uint16_t yd = y + j;
        if (mode & bm_flip_x) xd = x + w - i;
        if (mode & bm_flip_y) yd = y + h - j;
        drawPixel(xd, yd, pixelcolor);
      }
    }
  }
}




void  GxDEPG0290B::drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
    if (mode & bm_default) mode |= bm_flip_x | bm_invert;
    drawBitmapBM(bitmap, x, y, w, h, color, mode);
}

void GxDEPG0290B::drawBitmap(const uint8_t *bitmap, uint32_t size, int16_t mode)
{
    if (_current_page != -1) return;
    // example bitmaps are made for y-decrement, x-increment, for origin on opposite corner
    // bm_flip_x for normal display (bm_flip_y would be rotated)
    if (mode & bm_default) mode |= bm_flip_x;
    uint8_t ram_entry_mode = 0x03; // y-increment, x-increment for normal mode
    if ((mode & bm_flip_y) && (mode & bm_flip_x)) ram_entry_mode = 0x00; // y-decrement, x-decrement
    else if (mode & bm_flip_y) ram_entry_mode = 0x01; // y-decrement, x-increment
    else if (mode & bm_flip_x) ram_entry_mode = 0x02; // y-increment, x-decrement
    if (mode & bm_partial_update) {
        _using_partial_mode = true; // remember
        _Init_Part(ram_entry_mode);
        _writeCommand(0x24);
        for (uint32_t i = 0; i < GxDEPG0290B_BUFFER_SIZE; i++) {
            uint8_t data = 0xFF; // white is 0xFF on device
            if (i < size) {
          data = bitmap[i];
                if (mode & bm_invert) data = ~data;
            }
            _writeData(data);
        }
        _Update_Part();
        vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
        // update erase buffer
        _writeCommand(0x24);
        for (uint32_t i = 0; i < GxDEPG0290B_BUFFER_SIZE; i++) {
            uint8_t data = 0xFF; // white is 0xFF on device
            if (i < size) {

                data = bitmap[i];

                if (mode & bm_invert) data = ~data;
            }
            _writeData(data);
        }
        vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
        _PowerOff();
    } else {
        _using_partial_mode = false; // remember
        _Init_Full(ram_entry_mode);
        _writeCommand(0x24);
        for (uint32_t i = 0; i < GxDEPG0290B_BUFFER_SIZE; i++) {
            uint8_t data = 0xFF; // white is 0xFF on device
            if (i < size) {

                data = bitmap[i];

                if (mode & bm_invert) data = ~data;
            }
            _writeData(data);
        }
        _Update_Full();
        _PowerOff();
    }
}
#endif

void GxDEPG0290B::eraseDisplay(bool using_partial_update)
{
    if (_current_page != -1) return;
    if (using_partial_update) {
        _using_partial_mode = true; // remember
        _Init_Part(0x01);
        _writeCommand(0x24);
        for (uint32_t i = 0; i < GxDEPG0290B_BUFFER_SIZE; i++) {
            _writeData(0xFF);
        }
        _Update_Part();
        vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
        // update erase buffer
        _writeCommand(0x24);
        for (uint32_t i = 0; i < GxDEPG0290B_BUFFER_SIZE; i++) {
            _writeData(0xFF);
        }
        vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    } else {
        _using_partial_mode = false; // remember
        _Init_Full(0x01);
        _writeCommand(0x24);
        for (uint32_t i = 0; i < GxDEPG0290B_BUFFER_SIZE; i++) {
            _writeData(0xFF);
        }
        _Update_Full();
        _PowerOff();
    }
}

void GxDEPG0290B::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool using_rotation)
{
    if (_current_page != -1) return;
    if (using_rotation) _rotate(x, y, w, h);
    if (x >= GxDEPG0290B_WIDTH) return;
    if (y >= GxDEPG0290B_HEIGHT) return;
    uint16_t xe = gx_uint16_min(GxDEPG0290B_WIDTH, x + w) - 1;
    uint16_t ye = gx_uint16_min(GxDEPG0290B_HEIGHT, y + h) - 1;
    uint16_t xs_d8 = x / 8;
    uint16_t xe_d8 = xe / 8;
    _Init_Part(0x03);
    _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
    _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
    _waitBusy("0"); // needed ?
    _writeCommand(0x24);
    for (int16_t y1 = y; y1 <= ye; y1++) {
        for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++) {
            uint16_t idx = y1 * (GxDEPG0290B_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
            _writeData(~data);
        }
    }
    _Update_Part();
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    // update erase buffer
    _SetRamArea(xs_d8, xe_d8, y % 256, y / 256, ye % 256, ye / 256); // X-source area,Y-gate area
    _SetRamPointer(xs_d8, y % 256, y / 256); // set ram
    _waitBusy("0"); // needed ?
    _writeCommand(0x24);
    for (int16_t y1 = y; y1 <= ye; y1++) {
        for (int16_t x1 = xs_d8; x1 <= xe_d8; x1++) {
            uint16_t idx = y1 * (GxDEPG0290B_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
            _writeData(~data);
        }
    }
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
}

void GxDEPG0290B::_writeToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
    //Serial.printf("_writeToWindow(%d, %d, %d, %d, %d, %d)\n", xs, ys, xd, yd, w, h);
    // the screen limits are the hard limits
    if (xs >= GxDEPG0290B_WIDTH) return;
    if (ys >= GxDEPG0290B_HEIGHT) return;
    if (xd >= GxDEPG0290B_WIDTH) return;
    if (yd >= GxDEPG0290B_HEIGHT) return;
    w = gx_uint16_min(w, GxDEPG0290B_WIDTH - xs);
    w = gx_uint16_min(w, GxDEPG0290B_WIDTH - xd);
    h = gx_uint16_min(h, GxDEPG0290B_HEIGHT - ys);
    h = gx_uint16_min(h, GxDEPG0290B_HEIGHT - yd);
    uint16_t xds_d8 = xd / 8;
    uint16_t xde_d8 = (xd + w - 1) / 8;
    uint16_t yde = yd + h - 1;
    // soft limits, must send as many bytes as set by _SetRamArea
    uint16_t xse_d8 = xs / 8 + xde_d8 - xds_d8;
    uint16_t yse = ys + h - 1;
    _SetRamArea(xds_d8, xde_d8, yd % 256, yd / 256, yde % 256, yde / 256); // X-source area,Y-gate area
    _SetRamPointer(xds_d8, yd % 256, yd / 256); // set ram
    _waitBusy("0"); // needed ?
    _writeCommand(0x24);
    for (int16_t y1 = ys; y1 <= yse; y1++) {
        for (int16_t x1 = xs / 8; x1 <= xse_d8; x1++) {
            uint16_t idx = y1 * (GxDEPG0290B_WIDTH / 8) + x1;
            uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
            _writeData(~data);
        }
    }
}

void GxDEPG0290B::updateToWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h, bool using_rotation)
{
    if (using_rotation) {
        switch (getRotation()) {
        case 1:
            swap(xs, ys);
            swap(xd, yd);
            swap(w, h);
            xs = GxDEPG0290B_WIDTH - xs - w - 1;
            xd = GxDEPG0290B_WIDTH - xd - w - 1;
            break;
        case 2:
            xs = GxDEPG0290B_WIDTH - xs - w - 1;
            ys = GxDEPG0290B_HEIGHT - ys - h - 1;
            xd = GxDEPG0290B_WIDTH - xd - w - 1;
            yd = GxDEPG0290B_HEIGHT - yd - h - 1;
            break;
        case 3:
            swap(xs, ys);
            swap(xd, yd);
            swap(w, h);
            ys = GxDEPG0290B_HEIGHT - ys  - h - 1;
            yd = GxDEPG0290B_HEIGHT - yd  - h - 1;
            break;
        }
    }
    _Init_Part(0x03);
    _writeToWindow(xs, ys, xd, yd, w, h);
    _Update_Part();
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    // update erase buffer
    _writeToWindow(xs, ys, xd, yd, w, h);
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
}

void GxDEPG0290B::_sleep()
{
    _using_partial_mode = false;
    _writeCommand(0x10);
    _writeData(0x01);
    _waitBusy("power_off");
    gpio_bike_set_level(GPIO_ECRAN_CD,0);
    gpio_bike_set_level(GPIO_ECRAN_RESET,0);
}

void GxDEPG0290B::_writeCommand(uint8_t command)
{
    #if 0 
    if ((_busy >= 0) && digitalRead(_busy)) {
        String str = String("command 0x") + String(command, HEX);
        _waitBusy(str.c_str(), 100); // needed?
    }
    #endif
    IO.cmd(command);
}

void GxDEPG0290B::_writeData(uint8_t data)
{
    IO.data(data);
}

void GxDEPG0290B::_writeCommandData(const uint8_t *pCommandData, uint8_t datalen)
{
   
    IO.cmd(*pCommandData++);
    for (uint8_t i = 0; i < datalen - 1; i++) { // sub the command
        IO.data(*pCommandData++);
    }
}

void GxDEPG0290B::_waitBusy(const char *comment)
{
    if (debug_enabled)
  {
    ESP_LOGI(TAG, "_waitBusy for %s", comment);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (1)
  {
   
    if(gpio_bike_get_level(GPIO_ECRAN_BUSY)==0)
      break;
    vTaskDelay(1);
    if (esp_timer_get_time() - time_since_boot > 2000000)
    {
      if (debug_enabled) {
        ESP_LOGI(TAG, "Busy Timeout");
      }
      break;
    }
  }
}

void GxDEPG0290B::_setRamDataEntryMode(uint8_t em)
{
    const uint16_t xPixelsPar = GxDEPG0290B_X_PIXELS - 1;
    const uint16_t yPixelsPar = GxDEPG0290B_Y_PIXELS - 1;
    em = gx_uint16_min(em, 0x03);
    _writeCommand(0x11);
    _writeData(em);
    switch (em) {
    case 0x00: // x decrease, y decrease
        _SetRamArea(xPixelsPar / 8, 0x00, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
        _SetRamPointer(xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256); // set ram
        break;
    case 0x01: // x increase, y decrease : as in demo code
        _SetRamArea(0x00, xPixelsPar / 8, yPixelsPar % 256, yPixelsPar / 256, 0x00, 0x00);  // X-source area,Y-gate area
        _SetRamPointer(0x00, yPixelsPar % 256, yPixelsPar / 256); // set ram
        break;
    case 0x02: // x decrease, y increase
        _SetRamArea(xPixelsPar / 8, 0x00, 0x00, 0x00, yPixelsPar % 256, yPixelsPar / 256);  // X-source area,Y-gate area
        _SetRamPointer(xPixelsPar / 8, 0x00, 0x00); // set ram
        break;
    case 0x03: // x increase, y increase : normal mode
        _SetRamArea(0x00, xPixelsPar / 8, 0x00, 0x00, yPixelsPar % 256, yPixelsPar / 256);  // X-source area,Y-gate area
        _SetRamPointer(0x00, 0x00, 0x00); // set ram
        break;
    }
}

void GxDEPG0290B::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
    _writeCommand(0x44);
    _writeData(Xstart + 1 );
    _writeData(Xend + 1);
    _writeCommand(0x45);
    _writeData(Ystart);
    _writeData(Ystart1);
    _writeData(Yend);
    _writeData(Yend1);
}

void GxDEPG0290B::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1)
{
    _writeCommand(0x4e);
    _writeData(addrX + 1);
    _writeCommand(0x4f);
    _writeData(addrY);
    _writeData(addrY1);
}

void GxDEPG0290B::_wakeUp(void)
{
    gpio_bike_set_level(GPIO_ECRAN_RESET,0);
    vTaskDelay(10/portTICK_RATE_MS);
    gpio_bike_set_level(GPIO_ECRAN_RESET,1);
    vTaskDelay(10/portTICK_RATE_MS);
    _waitBusy("");

    // _writeCommand(0x22);
    // _writeData(0xc0);
    // _writeCommand(0x20);
    // _waitBusy("_PowerOn", power_on_time);
}

void GxDEPG0290B::_PowerOff(void)
{
    _writeCommand(0x10);
    _writeData(0x01);
}

void GxDEPG0290B::_InitDisplay(uint8_t em)
{
    _writeCommand(0x12);
    _waitBusy("");
    _setRamDataEntryMode(em);
}

void GxDEPG0290B::_Init_Full(uint8_t em)
{
    _wakeUp();
    _InitDisplay(em);
    //_PowerOn();
}

void GxDEPG0290B::_Init_Part(uint8_t em)
{
    _InitDisplay(em);
    _writeCommandData(LUTDefault_part, sizeof(LUTDefault_part));

    _writeCommand(0x3F);
    _writeData(0x22);
    _writeCommand(0x03);
    _writeData(0x17);

    _writeCommand(0x04);
    _writeData(0x41);
    _writeData(0xB0);
    _writeData(0x32);

    _writeCommand(0x2C);
    _writeData(0x36);

    _writeCommand(0x37);
    _writeData(0x00);
    _writeData(0x00);
    _writeData(0x00);
    _writeData(0x00);
    _writeData(0x00);
    _writeData(0x40);
    _writeData(0x00);
    _writeData(0x00);
    _writeData(0x00);
    _writeData(0x00);

    _writeCommand(0x3C);
    _writeData(0x80);

    _writeCommand(0x22);
    _writeData(0xC0);
    _writeCommand(0x20);
    _waitBusy("_Update_Part");

    //_PowerOn();
}

void GxDEPG0290B::_Update_Full(void)
{
    // _writeCommand(0x22);
    // _writeData(0xc4);
    _writeCommand(0x20);
    _waitBusy("_Update_Full");
    // _writeCommand(0xff);
}

void GxDEPG0290B::_Update_Part(void)
{
    _writeCommand(0x22);
    _writeData(0xCF);
    // _writeData(0x04);
    _writeCommand(0x20);
    _waitBusy("_Update_Part");
    // _writeCommand(0xff);
}

void GxDEPG0290B::drawPaged(void (*drawCallback)(void))
{
    if (_current_page != -1) return;
    _using_partial_mode = false;
    _Init_Full(0x03);
    _writeCommand(0x24);
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        fillScreen(EPD_WHITE);
        drawCallback();
        for (int16_t y1 = 0; y1 < GxDEPG0290B_PAGE_HEIGHT; y1++) {
            for (int16_t x1 = 0; x1 < GxDEPG0290B_WIDTH / 8; x1++) {
                uint16_t idx = y1 * (GxDEPG0290B_WIDTH / 8) + x1;
                uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
                _writeData(~data);
            }
        }
    }
    _current_page = -1;
    _Update_Full();
    _PowerOff();
}

void GxDEPG0290B::drawPaged(void (*drawCallback)(uint32_t), uint32_t p)
{
    if (_current_page != -1) return;
    _using_partial_mode = false;
    _Init_Full(0x03);
    _writeCommand(0x24);
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        fillScreen(EPD_WHITE);
        drawCallback(p);
        for (int16_t y1 = 0; y1 < GxDEPG0290B_PAGE_HEIGHT; y1++) {
            for (int16_t x1 = 0; x1 < GxDEPG0290B_WIDTH / 8; x1++) {
                uint16_t idx = y1 * (GxDEPG0290B_WIDTH / 8) + x1;
                uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
                _writeData(~data);
            }
        }
    }
    _current_page = -1;
    _Update_Full();
    _PowerOff();
}

void GxDEPG0290B::drawPaged(void (*drawCallback)(const void *), const void *p)
{
    if (_current_page != -1) return;
    _using_partial_mode = false;
    _Init_Full(0x03);
    _writeCommand(0x24);
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        fillScreen(EPD_WHITE);
        drawCallback(p);
        for (int16_t y1 = 0; y1 < GxDEPG0290B_PAGE_HEIGHT; y1++) {
            for (int16_t x1 = 0; x1 < GxDEPG0290B_WIDTH / 8; x1++) {
                uint16_t idx = y1 * (GxDEPG0290B_WIDTH / 8) + x1;
                uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
                _writeData(~data);
            }
        }
    }
    _current_page = -1;
    _Update_Full();
    _PowerOff();
}

void GxDEPG0290B::drawPaged(void (*drawCallback)(const void *, const void *), const void *p1, const void *p2)
{
    if (_current_page != -1) return;
    _using_partial_mode = false;
    _Init_Full(0x03);
    _writeCommand(0x24);
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        fillScreen(EPD_WHITE);
        drawCallback(p1, p2);
        for (int16_t y1 = 0; y1 < GxDEPG0290B_PAGE_HEIGHT; y1++) {
            for (int16_t x1 = 0; x1 < GxDEPG0290B_WIDTH / 8; x1++) {
                uint16_t idx = y1 * (GxDEPG0290B_WIDTH / 8) + x1;
                uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
                _writeData(~data);
            }
        }
    }
    _current_page = -1;
    _Update_Full();
    _PowerOff();
}

void GxDEPG0290B::_rotate(uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h)
{
    switch (getRotation()) {
    case 1:
        swap(x, y);
        swap(w, h);
        x = GxDEPG0290B_WIDTH - x - w - 1;
        break;
    case 2:
        x = GxDEPG0290B_WIDTH - x - w - 1;
        y = GxDEPG0290B_HEIGHT - y - h - 1;
        break;
    case 3:
        swap(x, y);
        swap(w, h);
        y = GxDEPG0290B_HEIGHT - y - h - 1;
        break;
    }
}

void GxDEPG0290B::drawPagedToWindow(void (*drawCallback)(void), uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (_current_page != -1) return;
    _rotate(x, y, w, h);
    if (!_using_partial_mode) {
        eraseDisplay(false);
        eraseDisplay(true);
    }
    _using_partial_mode = true;
    _Init_Part(0x03);
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        uint16_t yds = gx_uint16_max(y, _current_page * GxDEPG0290B_PAGE_HEIGHT);
        uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxDEPG0290B_PAGE_HEIGHT);
        if (yde > yds) {
            fillScreen(EPD_WHITE);
            drawCallback();
            uint16_t ys = yds % GxDEPG0290B_PAGE_HEIGHT;
            _writeToWindow(x, ys, x, yds, w, yde - yds);
        }
    }
    _Update_Part();
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    // update erase buffer
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        uint16_t yds = gx_uint16_max(y, _current_page * GxDEPG0290B_PAGE_HEIGHT);
        uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxDEPG0290B_PAGE_HEIGHT);
        if (yde > yds) {
            fillScreen(EPD_WHITE);
            drawCallback();
            uint16_t ys = yds % GxDEPG0290B_PAGE_HEIGHT;
            _writeToWindow(x, ys, x, yds, w, yde - yds);
        }
    }
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    _current_page = -1;
    _PowerOff();
}

void GxDEPG0290B::drawPagedToWindow(void (*drawCallback)(uint32_t), uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t p)
{
    if (_current_page != -1) return;
    _rotate(x, y, w, h);
    if (!_using_partial_mode) {
        eraseDisplay(false);
        eraseDisplay(true);
    }
    _using_partial_mode = true;
    _Init_Part(0x03);
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        uint16_t yds = gx_uint16_max(y, _current_page * GxDEPG0290B_PAGE_HEIGHT);
        uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxDEPG0290B_PAGE_HEIGHT);
        if (yde > yds) {
            fillScreen(EPD_WHITE);
            drawCallback(p);
            uint16_t ys = yds % GxDEPG0290B_PAGE_HEIGHT;
            _writeToWindow(x, ys, x, yds, w, yde - yds);
        }
    }
    _Update_Part();
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    // update erase buffer
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        uint16_t yds = gx_uint16_max(y, _current_page * GxDEPG0290B_PAGE_HEIGHT);
        uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxDEPG0290B_PAGE_HEIGHT);
        if (yde > yds) {
            fillScreen(EPD_WHITE);
            drawCallback(p);
            uint16_t ys = yds % GxDEPG0290B_PAGE_HEIGHT;
            _writeToWindow(x, ys, x, yds, w, yde - yds);
        }
    }
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    _current_page = -1;
    _PowerOff();
}

void GxDEPG0290B::drawPagedToWindow(void (*drawCallback)(const void *), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void *p)
{
    if (_current_page != -1) return;
    _rotate(x, y, w, h);
    if (!_using_partial_mode) {
        eraseDisplay(false);
        eraseDisplay(true);
    }
    _using_partial_mode = true;
    _Init_Part(0x03);
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        uint16_t yds = gx_uint16_max(y, _current_page * GxDEPG0290B_PAGE_HEIGHT);
        uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxDEPG0290B_PAGE_HEIGHT);
        if (yde > yds) {
            fillScreen(EPD_WHITE);
            drawCallback(p);
            uint16_t ys = yds % GxDEPG0290B_PAGE_HEIGHT;
            _writeToWindow(x, ys, x, yds, w, yde - yds);
        }
    }
    _Update_Part();
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    // update erase buffer
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        uint16_t yds = gx_uint16_max(y, _current_page * GxDEPG0290B_PAGE_HEIGHT);
        uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxDEPG0290B_PAGE_HEIGHT);
        if (yde > yds) {
            fillScreen(EPD_WHITE);
            drawCallback(p);
            uint16_t ys = yds % GxDEPG0290B_PAGE_HEIGHT;
            _writeToWindow(x, ys, x, yds, w, yde - yds);
        }
    }
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    _current_page = -1;
    _PowerOff();
}

void GxDEPG0290B::drawPagedToWindow(void (*drawCallback)(const void *, const void *), uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void *p1, const void *p2)
{
    if (_current_page != -1) return;
    _rotate(x, y, w, h);
    if (!_using_partial_mode) {
        eraseDisplay(false);
        eraseDisplay(true);
    }
    _using_partial_mode = true;
    _Init_Part(0x03);
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        uint16_t yds = gx_uint16_max(y, _current_page * GxDEPG0290B_PAGE_HEIGHT);
        uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxDEPG0290B_PAGE_HEIGHT);
        if (yde > yds) {
            fillScreen(EPD_WHITE);
            drawCallback(p1, p2);
            uint16_t ys = yds % GxDEPG0290B_PAGE_HEIGHT;
            _writeToWindow(x, ys, x, yds, w, yde - yds);
        }
    }
    _Update_Part();
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    // update erase buffer
    for (_current_page = 0; _current_page < GxDEPG0290B_PAGES; _current_page++) {
        uint16_t yds = gx_uint16_max(y, _current_page * GxDEPG0290B_PAGE_HEIGHT);
        uint16_t yde = gx_uint16_min(y + h, (_current_page + 1) * GxDEPG0290B_PAGE_HEIGHT);
        if (yde > yds) {
            fillScreen(EPD_WHITE);
            drawCallback(p1, p2);
            uint16_t ys = yds % GxDEPG0290B_PAGE_HEIGHT;
            _writeToWindow(x, ys, x, yds, w, yde - yds);
        }
    }
    vTaskDelay(GxDEPG0290B_PU_DELAY/portTICK_RATE_MS);
    _current_page = -1;
    _PowerOff();
}

void GxDEPG0290B::drawCornerTest(uint8_t em)
{
    if (_current_page != -1) return;
    _Init_Full(em);
    _writeCommand(0x24);
    for (uint32_t y = 0; y < GxDEPG0290B_HEIGHT; y++) {
        for (uint32_t x = 0; x < GxDEPG0290B_WIDTH / 8; x++) {
            uint8_t data = 0xFF;
            if ((x < 1) && (y < 8)) data = 0x00;
            if ((x > GxDEPG0290B_WIDTH / 8 - 3) && (y < 16)) data = 0x00;
            if ((x > GxDEPG0290B_WIDTH / 8 - 4) && (y > GxDEPG0290B_HEIGHT - 25)) data = 0x00;
            if ((x < 4) && (y > GxDEPG0290B_HEIGHT - 33)) data = 0x00;
            _writeData(data);
        }
    }
    _Update_Full();
    _PowerOff();
}

