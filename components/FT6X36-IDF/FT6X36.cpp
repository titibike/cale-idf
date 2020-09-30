#include "FT6X36.h"

FT6X36 *FT6X36::_instance = nullptr;
static const char *TAG = "i2c-touch";

FT6X36::FT6X36(int8_t intPin)
{
	_instance = this;

	i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)CONFIG_TOUCH_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)CONFIG_TOUCH_SDL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = CONFIG_I2C_MASTER_FREQUENCY;
    i2c_param_config(I2C_NUM_0, &conf);
    esp_err_t i2c_driver = i2c_driver_install(I2C_NUM_0, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	if (i2c_driver == ESP_OK) {
		printf("i2c_driver started correctly\n");
	} else {
		printf("i2c_driver error: %d\n", i2c_driver);
	}
	_intPin = intPin;
}

// Destructor should detach interrupt to the pin
FT6X36::~FT6X36()
{
	gpio_isr_handler_remove((gpio_num_t)CONFIG_TOUCH_INT);
}

void IRAM_ATTR FT6X36::isr(void* arg)
{
	if (_instance)
		_instance->onInterrupt();
}

bool FT6X36::begin(uint8_t threshold)
{
	uint8_t data_panel_id;
	readRegister8(FT6X36_REG_PANEL_ID, &data_panel_id);

	if (data_panel_id != FT6X36_VENDID) {
		ESP_LOGE(TAG,"FT6X36_VENDID does not match. Received:0x%x Expected:0x%x\n",data_panel_id,FT6X36_VENDID);
		return false;
		}
		ESP_LOGI(TAG, "\tDevice ID: 0x%02x", data_panel_id);

	uint8_t chip_id;
	readRegister8(FT6X36_REG_CHIPID, &chip_id);
	if (chip_id != FT6206_CHIPID && chip_id != FT6236_CHIPID && chip_id != FT6336_CHIPID) {
		ESP_LOGE(TAG,"FT6206_CHIPID does not match. Received:0x%x\n",chip_id);
		return false;
	}
	ESP_LOGI(TAG, "\tFound touch controller with Chip ID: 0x%02x", chip_id);
	
    // INT pin triggers the callback function on the Falling edge of the GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = 1ULL<<CONFIG_TOUCH_INT;  
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = (gpio_pullup_t)1; //  pull-up mode
    gpio_config(&io_conf);

	esp_err_t isr_service = gpio_install_isr_service(0);
    printf("ISR trigger install response: 0x%x %s\n", isr_service, (isr_service==0)?"ESP_OK":"");
    gpio_isr_handler_add((gpio_num_t)CONFIG_TOUCH_INT, isr, (void*) 1);

	writeRegister8(FT6X36_REG_DEVICE_MODE, 0x00);
	writeRegister8(FT6X36_REG_THRESHHOLD, threshold);
	writeRegister8(FT6X36_REG_TOUCHRATE_ACTIVE, 0x0E);
	return true;
}

void FT6X36::registerIsrHandler(void (*fn)())
{
	_isrHandler = fn;
}

void FT6X36::registerTouchHandler(void (*fn)(TPoint point, TEvent e))
{
	_touchHandler = fn;
}

uint8_t FT6X36::touched()
{
	uint8_t data_buf;
    esp_err_t ret = readRegister8(FT6X36_REG_NUM_TOUCHES, &data_buf);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error reading from device: %s", esp_err_to_name(ret));
	 }

	if (data_buf > 2)
	{
		data_buf = 0;
	}

	return data_buf;
}

void FT6X36::loop()
{
	while (_isrCounter > 0)
	{
		_isrCounter--;
		processTouch();
	}
}

void FT6X36::processTouch()
{
	readData();
	uint8_t n = 0;
	TRawEvent event = (TRawEvent)_touchEvent[n];
	TPoint point{_touchX[n], _touchY[n]};

	if (event == TRawEvent::PressDown)
	{
		_points[0] = point;
		_pointIdx = 1;
		_dragMode = false;
		_touchStartTime = esp_timer_get_time()/1000;
		fireEvent(point, TEvent::TouchStart);
	}
	else if (event == TRawEvent::Contact)
	{
		if (_pointIdx < 10)
		{
			_points[_pointIdx] = point;
			_pointIdx += 1;
		}
		if (!_dragMode && _points[0].aboutEqual(point) && (esp_timer_get_time()/1000) - _touchStartTime > 300)
		{
			_dragMode = true;
			fireEvent(point, TEvent::DragStart);
		}
		else if (_dragMode)
			fireEvent(point, TEvent::DragMove);

		fireEvent(point, TEvent::TouchMove);
	}
	else if (event == TRawEvent::LiftUp)
	{
		_points[9] = point;
		_touchEndTime = esp_timer_get_time()/1000;
		fireEvent(point, TEvent::TouchEnd);
		if (_dragMode)
		{
			fireEvent(point, TEvent::DragEnd);
			_dragMode = false;
		}
		if (_points[0].aboutEqual(point) && _touchEndTime - _touchStartTime <= 300)
		{
			fireEvent(point, TEvent::Tap);
			_points[0] = {0, 0};
			_touchStartTime = 0;
		}
	}
}

void FT6X36::onInterrupt()
{
	_isrCounter++;
	if (_isrHandler)
	{
		_isrHandler();
	}
}

uint8_t FT6X36::read8(uint8_t regName) {
	uint8_t buf;
	readRegister8(regName, &buf);
	return buf;
}

bool FT6X36::readData(void)
{
	uint8_t data_xy[4];         // 2 bytes X | 2 bytes Y
    uint8_t touch_pnt_cnt;      // Number of detected touch points

    readRegister8(FT6X36_REG_NUM_TOUCHES, &touch_pnt_cnt);
	if (touch_pnt_cnt==0) return 0;

	printf("TOUCHES:%d\n",touch_pnt_cnt);


    // Read X value
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (FT6X36_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd, FT6X36_REG_P1_XH, I2C_MASTER_ACK);

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (FT6X36_ADDR << 1) | I2C_MASTER_READ, true);

    i2c_master_read_byte(i2c_cmd, &data_xy[0], I2C_MASTER_ACK);     // reads FT6X36_P1_XH_REG
    i2c_master_read_byte(i2c_cmd, &data_xy[1], I2C_MASTER_NACK);    // reads FT6X36_P1_XL_REG
    i2c_master_stop(i2c_cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, i2c_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(i2c_cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error getting X coordinates: %s", esp_err_to_name(ret));
        // no touch detected
        return false;
    }

    // Read Y value
    i2c_cmd = i2c_cmd_link_create();

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (FT6X36_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd, FT6X36_REG_P1_YH, I2C_MASTER_ACK);

    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (FT6X36_ADDR << 1) | I2C_MASTER_READ, true);

    i2c_master_read_byte(i2c_cmd, &data_xy[2], I2C_MASTER_ACK);     // reads FT6X36_P1_YH_REG
    i2c_master_read_byte(i2c_cmd, &data_xy[3], I2C_MASTER_NACK);    // reads FT6X36_P1_YL_REG
    i2c_master_stop(i2c_cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, i2c_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(i2c_cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error getting Y coordinates: %s", esp_err_to_name(ret));
        return false;
    }

    _touchX[0] = ((data_xy[0] & FT6X36_MSB_MASK) << 8) | (data_xy[1] & FT6X36_LSB_MASK);
    _touchY[0] = ((data_xy[2] & FT6X36_MSB_MASK) << 8) | (data_xy[3] & FT6X36_LSB_MASK);
	_touchEvent[0] = data_xy[0] >> 5;
	
	if (CONFIG_FT6X36_DEBUG)
	printf("X: %d Y: %d T: %d\n", _touchX[0], _touchY[0], _touchEvent[0]);
	return true;
}

void FT6X36::writeRegister8(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd, FT6X36_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg , ACK_CHECK_EN);
	i2c_master_write_byte(cmd, value , ACK_CHECK_EN);
	i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

uint8_t FT6X36::readRegister8(uint8_t reg, uint8_t *data_buf)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd, FT6X36_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
	// Research: Why it's started a 2nd time here
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FT6X36_ADDR << 1) | I2C_MASTER_READ, true);

    i2c_master_read_byte(cmd, data_buf, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

	
	//FT6X36_REG_GESTURE_ID. Check if it can be read!
#if defined(FT6X36_DEBUG) && FT6X36_DEBUG==1
	printf("REG 0x%x: 0x%x\n",reg,ret);
#endif

	return ret;
}

void FT6X36::fireEvent(TPoint point, TEvent e)
{
	if (_touchHandler)
		_touchHandler(point, e);
}

void FT6X36::debugInfo()
{
	printf("            TH_DIFF: %d             CTRL: %d\n", read8(FT6X36_REG_FILTER_COEF), read8(FT6X36_REG_CTRL));
	printf("   TIMEENTERMONITOR: %d     PERIODACTIVE: %d\n", read8(FT6X36_REG_TIME_ENTER_MONITOR), read8(FT6X36_REG_TOUCHRATE_ACTIVE));
	printf("      PERIODMONITOR: %d     RADIAN_VALUE: %d\n", read8(FT6X36_REG_TOUCHRATE_MONITOR), read8(FT6X36_REG_RADIAN_VALUE));
	printf("  OFFSET_LEFT_RIGHT: %d   OFFSET_UP_DOWN: %d\n", read8(FT6X36_REG_OFFSET_LEFT_RIGHT), read8(FT6X36_REG_OFFSET_UP_DOWN));
	printf("DISTANCE_LEFT_RIGHT: %d DISTANCE_UP_DOWN: %d\n", read8(FT6X36_REG_DISTANCE_LEFT_RIGHT), read8(FT6X36_REG_DISTANCE_UP_DOWN));
	printf("      DISTANCE_ZOOM: %d           CIPHER: %d\n", read8(FT6X36_REG_DISTANCE_ZOOM), read8(FT6X36_REG_CHIPID));
	printf("             G_MODE: %d         PWR_MODE: %d\n", read8(FT6X36_REG_INTERRUPT_MODE), read8(FT6X36_REG_POWER_MODE));
	printf("             FIRMID: %d     FOCALTECH_ID: %d     STATE: %d\n", read8(FT6X36_REG_FIRMWARE_VERSION), read8(FT6X36_REG_PANEL_ID), read8(FT6X36_REG_STATE));
}