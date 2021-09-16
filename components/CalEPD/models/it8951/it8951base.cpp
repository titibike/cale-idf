#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "it8951/it8951base.h"

// Plasticlogic will replace Epd as baseclass for this models and have all common methods for all EPDs of this manufacturer
void It8951Base::initIO(bool debug) {
  IO.init(4, debug); // 4MHz frequency
  
  if (CONFIG_EINK_RST > -1) {
    IO.reset(5);
  }
  
}

/* uint8_t It8951Base::readTemperature()
{
  return IO.readTemp();
}

std::string It8951Base::readTemperatureString(char type) {
  uint8_t temperature = readTemperature();
  std::string temp = std::to_string(temperature);
  switch (type)
  {
  case 'c':
    temp = temp + " °C";
    break;
  case 'f':
  uint8_t fahrenheit = temperature * 9/5 + 32;
    temp = std::to_string(fahrenheit);
    temp += " °F";
    break;
  }
  return temp;
}
 */

// To be implemented
void It8951Base::_wakeUp(){
  
}

void It8951Base::_powerOn(void) {
   
}

void It8951Base::_waitBusy(const char* message, uint16_t busy_time){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();
  // On low is busy
  if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0) {
  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>busy_time)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  } else {
    vTaskDelay(busy_time/portTICK_RATE_MS); 
  }
}

// Called _poweroff in microEPD
void It8951Base::_powerOff(){
  
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
