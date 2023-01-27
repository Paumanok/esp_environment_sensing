/*
* Platform specific implementations of functions required by generic Bosche BME280 driver
* Author: Matthew Smith, github: @Paumanok
*/
#include <stdint.h>
#include <string.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "bme280.h"
#include "bme280_user.h"
#include "common.h"



void BME280_delay_us(uint32_t us)
{
    //actual driver only does milisecond delays
    //ie 1000 us for 1 ms delay
    //so lets just convert
    uint32_t ms = us/100;
	vTaskDelay(ms/portTICK_PERIOD_MS);
}


int8_t BME280_I2C_bus_write( uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt, uint8_t dev_addr)
{
    esp_err_t ret;
    uint8_t* write_buf = malloc(cnt + 1);
    memcpy((void*)write_buf, (const void*)(&reg_addr), sizeof(reg_addr));
    memcpy((void*)(write_buf+1), (const void*)reg_data, (size_t)cnt );
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, BME280_I2C_ADDR_PRIM, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    free(write_buf);
    return (int8_t)ret;
}

int8_t BME280_I2C_bus_read(uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt, uint8_t dev_addr)
{
    esp_err_t ret;
    ret =  i2c_master_write_read_device(I2C_MASTER_NUM, BME280_I2C_ADDR_PRIM, &reg_addr, 1, reg_data, cnt, I2C_MASTER_TIMEOUT_MS/ portTICK_PERIOD_MS);
    //ESP_LOGI("bus_read", "data: %x, count: %d", reg_data[0], cnt);
    return (int8_t)ret;
}

