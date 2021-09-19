/* Implement IoInterface for 4 wire SPI communication */
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "iointerface.h"

#pragma once

class EpdFastSpi : IoInterface
{
  public:
    spi_device_handle_t spi;

    void cmd(const uint8_t cmd) override;
    void data(uint8_t data) override;

    // Not implemented
    //void cmd16(const uint16_t data);
    void data16(const uint16_t data);
    void data16(const uint16_t *data, int len);
    
    // No data is used here (No DC pin)
    void data(const uint8_t *data, int len) override; 
    void reset(uint8_t millis) override;
    void init(uint8_t frequency, bool debug) override;

    // @deprecated and scheduled for deletion
    void csLow();
    void csHigh();

    void release();
    uint8_t readRegister(const uint8_t *data, int len);
    
    void waitForBusy();
  private:
    bool debug_enabled = true;
};