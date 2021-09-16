/* Implement IoInterface for 4 wire SPI communication with 2 CS pins */
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "iointerface.h"

#pragma once
// Instruction R/W bit set HIGH for data READ
#define EPD_REGREAD           0x80

class EpdFastSpi : IoInterface
{
  public:
    spi_device_handle_t spi;

    void cmd(const uint8_t cmd) override;
    void data(uint8_t data) override;

    void cmd16(const uint16_t data);
    void data16(const uint16_t data);
    
    // No data is used here (No DC pin)
    void data(const uint8_t *data, int len) override; 
    void reset(uint8_t millis) override;
    void init(uint8_t frequency, bool debug) override;

    void release();
    uint8_t readTemp();
    uint8_t readRegister(const uint8_t *data, int len);
    
    void waitForBusy();
  private:
    bool debug_enabled = true;
};