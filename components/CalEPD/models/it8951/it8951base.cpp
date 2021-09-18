#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "it8951/it8951base.h"

// Plasticlogic will replace Epd as baseclass for this models and have all common methods for all EPDs of this manufacturer
void It8951Base::initIO(bool debug) {
  IO.init(1, debug); // Gx uses 24 MHz frequency. 10 Mhz for read
  
  if (CONFIG_EINK_RST > -1) {
    printf("IO.reset(200)\n");
    IO.reset(200);
  }
  
}

void It8951Base::_InitDisplay() {
   // Long reset pulse
   printf("_InitDisplay\n");
   IO.reset(200);
   _waitBusy("Reset", reset_to_ready_time);
}

void It8951Base::_powerOff() {
  _IT8951StandBy();
  _waitBusy("_PowerOff", power_off_time);
  _power_is_on = false;
}

void It8951Base::_powerOn() {
  if (!_power_is_on)
  {
    _IT8951SystemRun();
    _waitBusy("_PowerOn", power_on_time);
  }
  _power_is_on = true;
}

void It8951Base::_IT8951WriteReg(uint16_t usRegAddr, uint16_t usValue)
{
  //Send Cmd , Register Address and Write Value
  _writeCommand16(IT8951_TCON_REG_WR);
  _writeData16(usRegAddr);
  _writeData16(usValue);
}

void It8951Base::_writeData16(uint16_t d)
{
  _waitBusy("_writeData16", default_wait_time);
  IO.csLow();
  IO.data16(0x0000);
  _waitBusy("_writeData16 preamble", default_wait_time);
  IO.data16(d);
  IO.csHigh();
}

void It8951Base::_writeCommand16(uint16_t c)
{
  char cmd[10];
  sprintf(cmd, "cmd(%x)", c);
  _waitBusy(cmd, default_wait_time);

  IO.csLow();
  IO.data16(0x6000); // preamble for write command
  /* uint8_t t16[2] = {0x60,0x00};
  IO.data(t16, sizeof(t16)); */
  _waitBusy("_writeCommand16 preamble", default_wait_time);


  IO.data16(c);
  IO.csHigh();
}

void It8951Base::_writeCommandData16(uint16_t c, const uint16_t* d, uint16_t n)
{
  IO.csLow();
  _writeCommand16(c);
  for (uint16_t i = 0; i < n; i++)
  {
    IO.data16(d[i]);
  }
  IO.csHigh();
}

void It8951Base::_IT8951SystemRun()
{
  _writeCommand16(IT8951_TCON_SYS_RUN);
}

void It8951Base::_IT8951StandBy()
{
  _writeCommand16(IT8951_TCON_STANDBY);
}

void It8951Base::_IT8951Sleep()
{
  _writeCommand16(IT8951_TCON_SLEEP);
}


void It8951Base::_waitBusy(const char* message, uint16_t busy_time){
  //if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
    // Add some margin
    vTaskDelay(2 / portTICK_RATE_MS);
  //}
  int64_t time_since_boot = esp_timer_get_time();
  // On low is busy
  if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) {
  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) {
      printf(" waited: %lld\n", esp_timer_get_time()-time_since_boot);
      break;
    }
    vTaskDelay(2 / portTICK_RATE_MS);
    if (esp_timer_get_time()-time_since_boot>busy_time)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  } else {
    printf("HRDY is high\n");
  }
}

void It8951Base::_IT8951SetVCOM(uint16_t vcom)
{
  _writeCommand16(USDEF_I80_CMD_VCOM);
  _waitBusy("_IT8951SetVCOM", default_wait_time);
  _writeData16(1);
  //Read data from Host Data bus
  _writeData16(vcom);
  _waitBusy("_IT8951SetVCOM", set_vcom_time);
}

// GFX functions
// display.print / println handling .TODO: Implement printf
size_t It8951Base::write(uint8_t v){
  Adafruit_GFX::write(v);
  return 1;
}
uint8_t It8951Base::_unicodeEasy(uint8_t c) {
  if (c<191 && c>131 && c!=176) { // 176 is °W 
    c+=64;
  }
  return c;
}

void It8951Base::print(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
}

void It8951Base::println(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter

     // _unicodeEasy will just sum 64 and get the right character when using umlauts and other characters:
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
   write(10); // newline
}

void It8951Base::newline() {
  write(10);
}
