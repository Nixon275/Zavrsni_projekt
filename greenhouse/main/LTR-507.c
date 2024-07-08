#include <stdio.h>
#include <rom/ets_sys.h>
#include "LTR-507.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static i2c_master_dev_handle_t dev_handle;
static uint16_t light_intensity;

static const char *TAG = "LTR-507";

void I2C_init(void)
{
    i2c_master_bus_config_t i2c_mst_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_PORT,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};

i2c_master_bus_handle_t bus_handle;
ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = LTR507_ADDRESS,
    .scl_speed_hz = I2C_MASTER_FREQ_HZ ,
};

i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);

}

uint16_t LTR507_getLightIntensity()
{   
    // First, check if the status is valid
    uint8_t addr1[] = {LTR507_ALS_PS_STATUS_REG}, status[1];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, addr1, sizeof(uint8_t), status, sizeof(uint8_t), -1));

    if (!(0x4 & status[0]))                         //data not ready
    {   
        ESP_LOGE(TAG, "Invalid LTR-507 reading.\n");
        return light_intensity;                     //returning old value
    }

    // Read ALS data and return it as a combined uint16_t
    uint8_t addr2[] = {LTR507_ALS_DATA_0_REG};
    uint8_t  reading[2];
    
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, addr2, sizeof(uint8_t), reading, sizeof(uint16_t), -1));
    //printf("reading[1] = %d, reading[0] = %d\n", reading[1], reading[0]);
    return light_intensity = (((uint16_t)reading[1]<<8) | reading[0]);

}

void LTR507_init(void)
{
    I2C_init();
    ALS_Enable(true);
    setALSGain(LTR507_ALS_GAIN_RANGE1);
    setALSBitWidth(LTR507_ALS_ADC_BIT_WIDTH_16BIT);
    setALSMeasRate(LTR507_ALS_MEASUREMENT_RATE_500MS);
}

void ALS_Enable(bool enable)
{
    uint8_t addr[] = {LTR507_ALS_PS_STATUS_REG}, buff[1];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, addr, sizeof(uint8_t), buff, sizeof(uint8_t), -1));
    if (enable)
        buff[0] |= 0x2; 
    else buff[0] |= 0xFD;

    uint8_t data[2] = {LTR507_ALS_CONTR_REG, buff[0]};
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, data, sizeof(uint16_t), -1));
        
}

void setALSGain (uint8_t gain_range)
{
    uint8_t addr[] = {LTR507_ALS_CONTR_REG}, buff[1];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, addr, sizeof(uint8_t), buff, sizeof(uint8_t), -1));
    buff[0] &= (~LTR507_ALS_GAIN_MASK); //clearing ALS Gain bits
    uint8_t data[2] = {LTR507_ALS_CONTR_REG, (buff[0] | (gain_range<<LTR507_ALS_GAIN_SHIFT))};
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, data, sizeof(uint16_t), -1));
}

void setALSBitWidth(uint8_t bit_width)
{
    uint8_t addr[] = {LTR507_ALS_CONTR_REG}, buff[1];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, addr, sizeof(uint8_t), buff, sizeof(uint8_t), -1));
    buff[0] &= (~LTR507_ALS_ADC_BIT_WIDTH_MASK); //clearing ALS BitWidth bits
    uint8_t data[2] = {LTR507_ALS_CONTR_REG, (buff[0] | (bit_width<<LTR507_ALS_ADC_BIT_WIDTH_SHIFT))};
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, data, sizeof(uint16_t), -1));
}

void setALSMeasRate(uint8_t meas_rate)
{
    uint8_t addr[] = {LTR507_ALS_CONTR_REG}, buff[1];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, addr, sizeof(uint8_t), buff, sizeof(uint8_t), -1));
    buff[0] &= (~LTR507_ALS_MEASURE_RATE_MASK); //clearing ALS MeasureRate bits
    uint8_t data[2] = {LTR507_ALS_CONTR_REG, (buff[0] | (meas_rate<<LTR507_ALS_MEASURE_RATE_SHIFT))};
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, data, sizeof(uint16_t), -1));
}


