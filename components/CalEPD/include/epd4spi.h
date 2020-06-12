/* Implement IoInterface for SPI communication with 4 Chip selects */
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "iointerface.h"

#ifndef epd4spi_h
#define epd4spi_h
class Epd4Spi : IoInterface
{
  public:
    spi_device_handle_t spi;
    // 4 different displays, same CLK & MOSI
    void cmdM1(const uint8_t cmd);
    void dataM1(uint8_t data);
    /* void cmdS1(const uint8_t cmd);
    void dataS1(uint8_t data);
    void cmdM2(const uint8_t cmd);
    void dataM2(uint8_t data);
    void cmdS2(const uint8_t cmd);
    void dataS2(uint8_t data);
    void readBusy(gpio_num_t pin); */

    void cmd(const uint8_t cmd) override;
    void data(uint8_t data) override;
    void data(const uint8_t *data, int len) override;
    void reset(uint8_t millis) override;
    void init(uint8_t frequency, bool debug) override;
  private:
    bool debug_enabled = true;
};
#endif
// Note: using override compiler will issue an error for "changing the type"
//       in case the type is changed.