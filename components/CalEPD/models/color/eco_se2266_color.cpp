#include "eco_se2266_color.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"
#include <math.h> 
#include "gpio_expander.h"
// Controller: Aurora MB EC2266
// Specification: https://www.pervasivedisplays.com/wp-content/uploads/2019/06/ApplicationNote_Small_Size_Mono_v01_181022-6.pdf

#define EPD_HOST SPI2_HOST

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

LUT_data ltb_custom2 = {
	{0xff, 0x8f},
	{0x07},
	{0x11},
	{0x0c},
	{0x01, 0x00, 5, 5, 0x01, 9, 1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x01, 0x55, 5, 5, 0x01, 9, 1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x01, 0xaa, 5, 5, 0x01, 9, 1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x01, 0x02, 5, 5, 0x01, 9, 1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x01, 0x01, 5, 5, 0x01, 9, 1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x27}
};

static const char *TAG="driver_ecran";
// Constructor
EcoSE2266::EcoSE2266(EpdSpi &dio) : Adafruit_GFX(EcoSE2266_WIDTH, EcoSE2266_HEIGHT),
                                    Epd(EcoSE2266_WIDTH, EcoSE2266_HEIGHT), IO(dio)
{
  ESP_LOGI(TAG,"EcoSE2266() constructor injects IO and extends Adafruit_GFX(%d,%d) Pix Buffer[%d]\n",
         EcoSE2266_WIDTH, EcoSE2266_HEIGHT, EcoSE2266_BUFFER_SIZE);
  ESP_LOGI(TAG,"\nAvailable heap after Epd bootstrap:%d\n", xPortGetFreeHeapSize());
}

//Initialize the display
void EcoSE2266::init(bool debug)
{
  debug_enabled = debug;
  int frequency=4;
  if (debug_enabled){
    ESP_LOGI(TAG,"EcoSE2266::init(debug:%d)\n", debug);
  }
  //Initialize SPI at 4MHz frequency. true for debug
  #if 0
 // IO.init(4, debug);
  #else
    //Initialize GPIOs direction & initial states
   
    gpio_bike_set_level(GPIO_CS_ECRAN,1);
    gpio_bike_set_level(GPIO_ECRAN_CD,1);
    gpio_bike_set_level(GPIO_ECRAN_RESET,1);
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
    
    /*
    if (debug_enabled) {
        frequency = 50;
        multiplier = 1;
    }*/
    

    #if 1 /* Not initialize because SD card already initialize it */
    //Initialize the SPI bus
    ret=spi_bus_initialize(EPD_HOST, &buscfg, EPD_HOST);
    ESP_ERROR_CHECK(ret);
    #endif
    
    
    #endif
  fillScreen(EPD_WHITE);
}

void EcoSE2266::addSpiDevice(){
  gpio_bike_t gpio_cs_ecran=getGpioBike(GPIO_CS_ECRAN);
  // max_transfer_sz   4Kb is the defaut SPI transfer size if 0
    // debug: 50000  0.5 Mhz so we can sniff the SPI commands with a Slave
    uint16_t multiplier = 1000;
    uint16_t frequency=4;
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
    _previous_buffer[x] = red;
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
  ESP_LOGI(TAG,"Wake up  \n");
  uint8_t data[]={0};
  vTaskDelay(10/portTICK_RATE_MS); // delay_ms1
  gpio_bike_set_level(GPIO_ECRAN_RESET,1);
  ets_delay_us(4500);
  gpio_bike_set_level(GPIO_ECRAN_RESET,1);
  vTaskDelay(10/portTICK_RATE_MS); // delay_ms3 5ms
  gpio_bike_set_level(GPIO_ECRAN_RESET,0);
  ets_delay_us(4500);
  gpio_bike_set_level(GPIO_ECRAN_RESET,1);
  vTaskDelay(10/portTICK_RATE_MS); // delay_ms5 5ms

  gpio_bike_set_level(GPIO_CS_ECRAN,1);
  
  IO.cmd(0x00); 
  data[0]=0x0e;
  IO.data(data,1); // Soft-reset
  ets_delay_us(4500);

  IO.cmd(0xe5);
  data[0]=0x19;
  IO.data(data,1);// Input Temperature 0°C = 0x00, 22°C = 0x16, 25°C = 0x19

  
  IO.cmd(0xe0);// Active Temperature
  data[0]=0x02;
  IO.data(data,1);
  
  uint8_t data0[] = {0xcf , 0x8d};
  IO.cmd(0x00);
  IO.data(data0,2); // PSR
 
  /* uint8_t data4[] = {0x07};
  IO.cmd(0x50);
  IO.data(data4,1);// Vcom and data interval setting
  */
  //_sendIndexData(0x50, data4, 1); 

  #if 1
  IO.cmd(0x04);
  data[0]=0x0;
  IO.data(data,1);
  _waitBusy("_wakeUp power on");
  #endif
  
}

void EcoSE2266::update()
{
 
  //memcpy(_color,redBuffer,EcoSE2266_BUFFER_SIZE);
  uint8_t * nextBuffer = _buffer;
  uint8_t * previousBuffer = _previous_buffer;
  if (!_using_partial_mode){
  _wakeUp();
  _using_partial_mode = false;
  }
  
  uint8_t data[]={0};
  // send first frame
  IO.cmd(0x10);       
  IO.data(nextBuffer,EcoSE2266_BUFFER_SIZE);
// send second frame
  IO.cmd(0x13); 
  for (int i=0;i<EcoSE2266_BUFFER_SIZE;i++){      
    IO.data(0x00);
  }

  IO.cmd(0x04); //DCDC Power on
  data[0]=0;
  IO.data(data,1);
  _waitBusy("power on");

  IO.cmd(0x12); //refresh
  data[0]=0x0;
  IO.data(data,1);
  vTaskDelay(50/portTICK_RATE_MS);
  _waitBusy("refresh");
}

// Global Update function
//		Implements global update functionality on either small/mid EPD
//		- INPUT:
//			- two image data (either BW and 0x00 or BW and BWR types)

void EcoSE2266::globalUpdate(const uint8_t * data1s, const uint8_t * data2s){
  uint8_t data[]={0};
  // send first frame
  IO.cmd(0x10);       
  IO.data(data1s,EcoSE2266_BUFFER_SIZE);
// send second frame
  IO.cmd(0x13);       
  IO.data(data2s,EcoSE2266_BUFFER_SIZE);

  IO.cmd(0x04); //DCDC Power on
  data[0]=0;
  IO.data(data,1);
  _waitBusy("power on");

  IO.cmd(0x12); //refresh
  data[0]=0x0;
  IO.data(data,1);
  vTaskDelay(50/portTICK_RATE_MS);
  _waitBusy("refresh");

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
   
    if(gpio_bike_get_level(GPIO_ECRAN_BUSY)==1)
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

void EcoSE2266::_sleep()
{
  uint8_t data[]={0x0};
  IO.cmd(0x02);
  IO.data(data,1);
  _waitBusy("power_off");

  gpio_bike_set_level(GPIO_ECRAN_CD,0);
  gpio_bike_set_level(GPIO_ECRAN_RESET,0);
  
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
  #if 0
  if (color) {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    } else {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    }
  #else 
  // This is the trick to draw colors right. Genious Jean-Marc
  if (color) {
    _buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
    } else {
    _buffer[i] = (_buffer[i] | (1 << (7 - x % 8)));
    }

  #endif
}


void EcoSE2266::initFastUpdate(LUT_data ltc){
    uint8_t temp=16; 

    IO.cmd(0x00);  //Panel setting for fast partial
    IO.data(ltc.panelSet,2);

    IO.cmd(0X50);  //VCOM AND DATA INTERVAL SETTING
    IO.data(ltc.vcomIntrval,1); 

    IO.cmd(0x30); //PLL framerate
    IO.data(ltc.PLLframert,1);   

    IO.cmd(0x82);  //vcom_DC setting
    IO.data(ltc.vcomDC,1);
  
  // send LUT
   IO.cmd(0x20);  
    IO.data(ltc.lutC,temp); //VCOM LUT(LUTC)
  IO.cmd(0x23);  
    IO.data(ltc.lutWb_W,temp); //W2B(LUTWB / LUTW)
	
IO.cmd(0x22);  
    IO.data(ltc.lutBW_R,temp); //B2W((LUTBW / LUTR)
	
	IO.cmd(0x21);  
    IO.data(ltc.lutWW,temp);//W2W(LUTWW) not available in BW mode  
  
    IO.cmd(0x24);  
    IO.data(ltc.lutBB_B,temp); //B2B(LUTBB / LUTB)

    if (debug_enabled) { 
      ESP_LOGI(TAG,"initfastupdate() LUT\n");
    }
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

void EcoSE2266::_clearWindow(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
  ESP_LOGI(TAG,"w :%d h :%d x_start:%0x x_end:%0x y_start:%0x y_end:%0x\n",w,h,xs,xd,ys,yd);

  uint8_t datacmd[]={0};
  int size_window=(w*h)/8; 

  uint16_t idx=0;
  int cpt=0; 
  // the screen limits are the hard limits
  if (xs >= EcoSE2266_WIDTH) return;
  if (ys >= EcoSE2266_HEIGHT) return;
  if (xd >= EcoSE2266_WIDTH) return;
  if (yd >= EcoSE2266_HEIGHT) return;

  uint8_t windowSource[2] = {};	// HRST, HRED
	uint16_t windowGate[2] = {};	// VRST, VRED

  
   w = gx_uint16_min(w + 7, EcoSE2266_WIDTH - xd) + (xd % 8);
   h = gx_uint16_min(h, EcoSE2266_HEIGHT - yd);
  
/* Coordinates for set window*/
uint16_t h_start=((xs)/8);
uint16_t h_end=  ( ((w+xs-8) /8) );
  uint16_t v_start= ys; 
   uint16_t v_end=h+ys;
 ESP_LOGI(TAG,"\n H_start:%0x H_end:%0x V_start:%0x V_end:%0x \n",h_start,h_end,v_start,v_end);

  uint16_t xe = ((xs +w )/ 8);

  windowSource[0]=h_start;
  windowSource[1]=h_end;
  windowGate[0]=v_start;
  windowGate[1]=v_end;

  
  uint8_t PU_data[7];
		PU_data[0] = (windowSource[0]<<3)&0xf8;     // source start
		PU_data[1] = (windowSource[1]<<3)|0x07;     // source end
		PU_data[2] = (windowGate[0]>>8)&0x01;       // Gate start MSB
		PU_data[3] = windowGate[0]&0xff;            // Gate start LSB
		PU_data[4] = (windowGate[1]>>8)&0x01;       // Gate end MSB
		PU_data[5] = windowGate[1]&0xff;            // Gate end LSB
		PU_data[6] = 0x01;
    
    
    
    ESP_LOGI(TAG,"PU_data:\n");
    for (int i=0;i<7;i++){
      ESP_LOGI(TAG,"PU_data[%d]:%d\n",i,PU_data[i]);
    }
  
    IO.cmd(0x90);
    IO.data(&PU_data[0],7);
		
    IO.cmd(0x91);
    IO.data(&PU_data[0],0); //0x91 doesn’t have data

  /* end Example code */
  
/* **/

  #if 1

  /* Construct the data buffer to send */

  IO.cmd(0x10);     
  
  for (uint16_t y1 = ys; y1 <= ys+h; y1++)
  {
    for (uint16_t x1 = xs/8; x1 < xe; x1++)
    {
       idx = y1 * (EcoSE2266_WIDTH / 8 ) + x1;
        IO.data(0x00);
    }
  }

  IO.cmd(0x13);
  for (uint16_t y1 = ys; y1 <= ys+h; y1++)
  {
    for (uint16_t x1 = xs/8; x1 < xe; x1++)
    {
       idx = y1 * (EcoSE2266_WIDTH / 8 ) + x1;
        IO.data(0x00);
    }
  }


      IO.cmd(0x50);
      IO.data(ltb.vcomIntrval,1); 

      IO.cmd(0x12);
      datacmd[0]=0x0;
      IO.data(datacmd,1);

      //memcpy(_previous_buffer,_buffer,EcoSE2266_BUFFER_SIZE);
      vTaskDelay(20/portTICK_RATE_MS);
      _waitBusy("refresh");
  #endif
  ESP_LOGI(TAG,"Data window : \n");
  int a=0;
  #if 0
  for(int b=0;b<EcoSE2266_BUFFER_SIZE;b++){
    if(a >= 4186){
      a++;
    }
    ESP_LOGI(TAG,"data[%d]:%0x buffer[%d]:%0x\n",a,data[a],b,_buffer[b]);
  }
  #endif
  ESP_LOGI(TAG,"\n");
  vTaskDelay(pdMS_TO_TICKS(2));
}


void EcoSE2266::_writeToWindow(uint8_t command, uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t w, uint16_t h)
{
  ESP_LOGI(TAG,"w :%d h :%d x_start:%0x x_end:%0x y_start:%0x y_end:%0x\n",w,h,xs,xd,ys,yd);

  uint8_t datacmd[]={0};
  int size_window=(w*h)/8; 

  uint16_t idx=0;
  int cpt=0; 


  // the screen limits are the hard limits
  if (xs >= EcoSE2266_WIDTH) return;
  if (ys >= EcoSE2266_HEIGHT) return;
  if (xd >= EcoSE2266_WIDTH) return;
  if (yd >= EcoSE2266_HEIGHT) return;

  uint8_t windowSource[2] = {};	// HRST, HRED
	uint16_t windowGate[2] = {};	// VRST, VRED

  
   w = gx_uint16_min(w + 7, EcoSE2266_WIDTH - xd) + (xd % 8);
   h = gx_uint16_min(h, EcoSE2266_HEIGHT - yd);
  
/* Coordinates for set window*/
uint16_t h_start=((xs)/8);
uint16_t h_end=  ( ((w+xs-8) /8) );
  uint16_t v_start= ys; 
   uint16_t v_end=h+ys;
 ESP_LOGI(TAG,"\n H_start:%0x H_end:%0x V_start:%0x V_end:%0x \n",h_start,h_end,v_start,v_end);

  uint16_t xe = ((xs +w )/ 8);

  windowSource[0]=h_start;
  windowSource[1]=h_end;
  windowGate[0]=v_start;
  windowGate[1]=v_end;

  
  uint8_t PU_data[7];
		PU_data[0] = (windowSource[0]<<3)&0xf8;     // source start
		PU_data[1] = (windowSource[1]<<3)|0x07;     // source end
		PU_data[2] = (windowGate[0]>>8)&0x01;       // Gate start MSB
		PU_data[3] = windowGate[0]&0xff;            // Gate start LSB
		PU_data[4] = (windowGate[1]>>8)&0x01;       // Gate end MSB
		PU_data[5] = windowGate[1]&0xff;            // Gate end LSB
		PU_data[6] = 0x01;
    
    
    
    ESP_LOGI(TAG,"PU_data:\n");
    for (int i=0;i<7;i++){
      ESP_LOGI(TAG,"PU_data[%d]:%d\n",i,PU_data[i]);
    }
  
    IO.cmd(0x90);
    IO.data(&PU_data[0],7);
		
    IO.cmd(0x91);
    IO.data(&PU_data[0],0); //0x91 doesn’t have data

  /* end Example code */
  
/* **/

  #if 1

  /* Construct the data buffer to send */

  IO.cmd(0x10);     
  #if 1
  ESP_LOGI(TAG,"data 0x10 \r\n");
  for (uint16_t y1 = ys; y1 <= ys+h; y1++)
  {
    for (uint16_t x1 = xs/8; x1 < xe; x1++)
    {
       idx = y1 * (EcoSE2266_WIDTH / 8 ) + x1;
       uint8_t previous_data = (idx < sizeof(_buffer)) ? _previous_buffer[idx] : 0x00;
      IO.data(previous_data);
    }
  }
  #endif

  IO.cmd(0x13);
  for (uint16_t y1 = ys; y1 <= ys+h; y1++)
  {
    for (uint16_t x1 = xs/8; x1 < xe; x1++)
    {
       idx = y1 * (EcoSE2266_WIDTH / 8 ) + x1;
       uint8_t data = (idx < sizeof(_buffer)) ? _buffer[idx] : 0x00;
        IO.data(data);
    }
  }


      IO.cmd(0x50);
      IO.data(ltb.vcomIntrval,1); 

      IO.cmd(0x12);
      datacmd[0]=0x0;
      IO.data(datacmd,1);

      memcpy(_previous_buffer,_buffer,EcoSE2266_BUFFER_SIZE);
      vTaskDelay(20/portTICK_RATE_MS);
      _waitBusy("refresh");
  #endif
  ESP_LOGI(TAG,"Data window : \n");
  int a=0;
  #if 0
  for(int b=0;b<EcoSE2266_BUFFER_SIZE;b++){
    if(a >= 4186){
      a++;
    }
    ESP_LOGI(TAG,"data[%d]:%0x buffer[%d]:%0x\n",a,data[a],b,_buffer[b]);
  }
  #endif
  ESP_LOGI(TAG,"\n");
  vTaskDelay(pdMS_TO_TICKS(2));
}

void EcoSE2266::updateWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{

  ESP_LOGI(TAG,"\n before rotate x:%d y:%d w:%d h : %d \n",x, y, w, h);

  _rotate(x, y, w, h);
  ESP_LOGI(TAG,"\n after rotate x:%d y:%d w:%d h : %d \n",x, y, w, h);
  // Only if sleep state is true:
  //if (!_using_partial_mode) _wakeUp();
  _using_partial_mode = true;
  //_clearWindow(x, y, x, y, w, h);
  fastUpdateInit();

  /* Try to replace fast update init */


  _writeToWindow(0x15, x, y, x, y, w, h);
  _waitBusy("updateWindow");
  /*
  _refreshWindow(x, y, w, h);   
  _waitBusy("updateWindow");
  _writeToWindow(0x14, x, y, x, y, w, h);*/
}



// Fast Update function
//		Implements fast update functionality
//		- INPUT:
//			- array of image data to iterate on
//			- size of array
void EcoSE2266::fastUpdate(){
  uint8_t data[]={0};

  //First or previous frame
  IO.cmd(0x10);       
  IO.data(_previous_buffer,EcoSE2266_BUFFER_SIZE);
  
  //Second or new frame
  IO.cmd(0x13);     
  IO.data(_buffer,EcoSE2266_BUFFER_SIZE);

  IO.cmd(0x50);
  IO.data(ltb.vcomIntrval,1); 

  IO.cmd(0x12);
  data[0]=0x0;
  IO.data(data,1);
  
  memcpy(_previous_buffer,_buffer,EcoSE2266_BUFFER_SIZE);
  vTaskDelay(50/portTICK_RATE_MS);
  _waitBusy("refresh");
}

/**
 * @brief Init fast update 
 * 
 */
void EcoSE2266::fastUpdateInit(){
  //Turn off DC/DC
  ESP_LOGI(TAG,"\nfastUpdateInit\n");
  uint8_t data[]={0};
  IO.cmd(0x02);
  data[0]=0x00;
  IO.data(data,1); 
  _waitBusy("fastTest");


   IO.cmd(0x00); 
  data[0]=0x0e;
  IO.data(data,1); // Soft-reset

  initFastUpdate(ltb);

  //DC DC Power on 
  IO.cmd(0x04);
  data[0]=0x00;
  IO.data(data,1); 
  _waitBusy("DCDC");
}

// Fast Update function
//		Implements fast update functionality
//		- INPUT:
//			- array of image data to iterate on
//			- size of array
void EcoSE2266::fastUpdateTest(const unsigned char* fastImgSet[], uint8_t fastImgSize,uint8_t numLoops){
  //Turn off DC/DC
  fastUpdateInit();

  memcpy(_previous_buffer,fastImgSet[0],EcoSE2266_BUFFER_SIZE);
  uint8_t ii = 0;
	while (ii < numLoops)
	{
    #if 0
		for (uint8_t j = 0; j < fastImgSize; j++)
		{
      #if 0
		  //First or previous frame
      uint8_t data[]={0};
			IO.cmd(0x10);        
      IO.data(fastImgSet[j],EcoSE2266_BUFFER_SIZE);
      
      //Second or new frame
			IO.cmd(0x13);     
      IO.data(fastImgSet[j+1],EcoSE2266_BUFFER_SIZE);
      
      IO.cmd(0x50);
      IO.data(ltb.data4,1); 

		  IO.cmd(0x12);
      data[0]=0x0;
      IO.data(data,1);
      vTaskDelay(50/portTICK_RATE_MS);
      _waitBusy("refresh");
      #elif 0
      //memcpy(_previous_buffer,fastImgSet[j],EcoSE2266_BUFFER_SIZE);

      memcpy(_buffer,fastImgSet[j+1],EcoSE2266_BUFFER_SIZE);
      if(j>0){
        ESP_LOGI(TAG,"previous buff ==fastImgSet[j-1] ? : %d\n ", memcmp(_previous_buffer,fastImgSet[j-1],EcoSE2266_BUFFER_SIZE));
      }
      fastUpdate( );
      
      ESP_LOGI(TAG,"previous buff ==fastImgSet[j] ? : %d\n ", memcmp(_previous_buffer,fastImgSet[j],EcoSE2266_BUFFER_SIZE));
      
      #elif 0
      ESP_LOGI(TAG,"previous buff ==fastImgSet[j] ? : %d\n ", memcmp(_previous_buffer,fastImgSet[j],EcoSE2266_BUFFER_SIZE));
      #endif
			
		}
		
    #endif
    setCursor(20,30);
    setTextColor(EPD_BLACK);
    println("Hello !!");
    fastUpdate( );
  for (int i=0;i<20;i++){
    drawPixel(i,10,EPD_BLACK);
    fastUpdate( );
  }
  ii++;
	}
  

}


// Look-up table update function
//		Enables fast update functionality
//		- INPUT:
//			- array of image data to iterate on
//			- size of array
void EcoSE2266::updateLUT(LUT_data *ltc)
{
	ltb = *ltc;
}


void EcoSE2266::initPartialUpdate(uint8_t partialImgConfig[5]){
/* Example code*/

#if 0
  /* simulate x,y ,w ,h  0x07, 0x0b, 0x32 ,0x6a */
uint16_t xs=56; 
uint16_t ys=50;  
uint16_t xd=96; 
uint16_t yd=106;  
uint16_t w=40; 
uint16_t h=56; 
   uint8_t windowSource[2] = {};	// HRST, HRED
	uint8_t windowGate[2] = {};	// VRST, VRED

  uint8_t hrst=floor(xs/8);
   //w = gx_uint16_min(w + 7, EcoSE2266_WIDTH - xd) + (xd % 8);
   //h = gx_uint16_min(h, EcoSE2266_HEIGHT - yd);
  uint8_t hred=floor( ( (w+xs)/8) -1 );
  uint vrst= ys; 
  uint vred=h+ys;
  
  windowSource[0]=hrst;
  windowSource[1]=hred;
  windowGate[0]=vrst;
  windowGate[1]=vred;

  ESP_LOGI(TAG,"w :%d h :%d HRST:%0x HRED:%0x VRST:%0x VRED:%0x\n",w,h,hrst,hred,vrst,vred);
  uint8_t PU_data[7];
		PU_data[0] = (windowSource[0]<<3)&0xf8;     // source start
		PU_data[1] = (windowSource[1]<<3)|0x07;     // source end
		PU_data[2] = (windowGate[0]>>8)&0x01;       // Gate start MSB
		PU_data[3] = windowGate[0]&0xff;            // Gate start LSB
		PU_data[4] = (windowGate[1]>>8)&0x01;       // Gate end MSB
		PU_data[5] = windowGate[1]&0xff;            // Gate end LSB
		PU_data[6] = 0x01;
    
    IO.cmd(0x90);
    IO.data(&PU_data[0],7);
		
    IO.cmd(0x91);
    IO.data(&PU_data[0],0); //0x91 doesn’t have data

  
  /* end Example code */
#else

  uint8_t windowSource[2] = {};	// HRST, HRED
	uint8_t windowGate[2] = {};	// VRST, VRED
	
	memcpy(windowSource, &partialImgConfig[1], sizeof(windowSource));
	memcpy(windowGate, &partialImgConfig[3], sizeof(windowGate));

  uint8_t PU_data[7];
		PU_data[0] = (windowSource[0]<<3)&0xf8;     // source start
		PU_data[1] = (windowSource[1]<<3)|0x07;     // source end
		PU_data[2] = (windowGate[0]>>8)&0x01;       // Gate start MSB
		PU_data[3] = windowGate[0]&0xff;            // Gate start LSB
		PU_data[4] = (windowGate[1]>>8)&0x01;       // Gate end MSB
		PU_data[5] = windowGate[1]&0xff;            // Gate end LSB
		PU_data[6] = 0x01;
    
    IO.cmd(0x90);
    IO.data(&PU_data[0],7);
		
    IO.cmd(0x91); //0x91 doesn’t have data
#endif
} 

#if 0
void EcoSE2266::initHWPartialUpdate(){
  //Turn off DC/DC
  uint8_t data[]={0};
  IO.cmd(0x02);
  data[0]=0x00;
  IO.data(data,1); 
  _waitBusy("fastTest");


   IO.cmd(0x00); 
  data[0]=0x0e;
  IO.data(data,1); // Soft-reset

  initFastUpdate(ltb);

  //DC DC Power on 
  IO.cmd(0x04);
  data[0]=0x00;
  IO.data(data,1); 
  _waitBusy("DCDC");
}
#endif
void EcoSE2266::partialUpdateTest(const unsigned char* partialImgSet[], uint8_t partialImgConfig[5], long windowSize, uint8_t numLoops)
{
	//Turn off DC/DC
  fastUpdateInit();
	//initPartialUpdate(partialImgConfig);

  updateWindow(56,50,40,56);
	uint8_t i=0;
	while (i < numLoops)
	{
		for (uint8_t j=0; j < partialImgConfig[0] -1; j++)
		{

       //First or previous frame
      uint8_t data[]={0};
			IO.cmd(0x10);        
      IO.data(partialImgSet[j], windowSize);
      
      //Second or new frame
			IO.cmd(0x13);     
      IO.data(partialImgSet[j+1], windowSize);
      
      IO.cmd(0x50);
      IO.data(ltb.vcomIntrval,1); 

		  IO.cmd(0x12);
      data[0]=0x0;
      IO.data(data,1);
      vTaskDelay(50/portTICK_RATE_MS);
      _waitBusy("refresh");
			
		}
		i++;
	}
}

void EcoSE2266::printBuffer(){
  ESP_LOGI(TAG,"Buffer : \n");
  for (int i=0; i<EcoSE2266_BUFFER_SIZE;i++){
    if (_buffer[i]!=0){
    ESP_LOGI(TAG,"Buffer[%d]:%0x\n",i,_buffer[i]);
    }

  }
  ESP_LOGI(TAG,"End Buffer \n");
}

#if 0
void EcoSE2266::partialUpdate(const unsigned char* partialImgSet[], uint8_t partialImgConfig[5], long windowSize, uint8_t numLoops)
{
	
    uint8_t data[]={0};

  //First or previous frame
  IO.cmd(0x10);       
  IO.data(_previous_buffer,windowSize);
  
  //Second or new frame
  IO.cmd(0x13);     
  IO.data(_buffer,windowSize);

  IO.cmd(0x50);
  IO.data(ltb.vcomIntrval,1); 

  IO.cmd(0x12);
  data[0]=0x0;
  IO.data(data,1);
  
  memcpy(_previous_buffer,_buffer,EcoSE2266_BUFFER_SIZE);
  vTaskDelay(50/portTICK_RATE_MS);
  _waitBusy("refresh");
	uint8_t i=0;
	while (i < numLoops)
	{
		for (uint8_t j=0; j < partialImgConfig[0] -1; j++)
		{
      #if 0
       //First or previous frame
      uint8_t data[]={0};
			IO.cmd(0x10);        
      IO.data(partialImgSet[j], windowSize);
      
      //Second or new frame
			IO.cmd(0x13);     
      IO.data(partialImgSet[j+1], windowSize);
      
      IO.cmd(0x50);
      IO.data(ltb.vcomIntrval,1); 

		  IO.cmd(0x12);
      data[0]=0x0;
      IO.data(data,1);
      vTaskDelay(50/portTICK_RATE_MS);
      _waitBusy("refresh");
			#else 
      //First or previous frame
      IO.cmd(0x10);       
      IO.data(_previous_buffer,windowSize);
      
      //Second or new frame
      IO.cmd(0x13);     
      IO.data(_buffer,windowSize);

      IO.cmd(0x50);
      IO.data(ltb.vcomIntrval,1); 

      IO.cmd(0x12);
      data[0]=0x0;
      IO.data(data,1);
      
      memcpy(_previous_buffer,_buffer,EcoSE2266_BUFFER_SIZE);
      vTaskDelay(50/portTICK_RATE_MS);
      _waitBusy("refresh");
      #endif
		}
		i++;
	}
}
#endif