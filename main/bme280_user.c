/*
User implementations of bme280 functions
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

// esp_err_t user_read_byte(uin8_t, uint8_t *data)
// {
//     return i2c_master_read_device(I2C_MASTER_NUM, BME280_I2C_ADDR_PRIM, &reg_addr, 1, 1, I2C_MASTER_TIMEOUT_MS/ portTICK_PERIOD_MS);
// }

// esp_err_t register_write_byte(uint8_t reg_addr, uint8_t data)
// {
//     int ret;
//     uint8_t write_buf[2] = {reg_addr, data};

//     ret = i2c_master_write_to_device(I2C_MASTER_NUM, BME280_I2C_ADDR_PRIM, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

//     return ret;
// }

#define I2C_MASTER_TIMEOUT_MS 1000

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
    //set read mode for desired reg
    //ret = i2c_master_write_to_device(I2C_MASTER_NUM, BME280_I2C_ADDR_PRIM, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    ////////////////////////////////i2c_num, dev_addr, write_buffer, write_size, read_buffer, read_size, ticks_to_wait
    ret =  i2c_master_write_read_device(I2C_MASTER_NUM, BME280_I2C_ADDR_PRIM, &reg_addr, 1, reg_data, cnt, I2C_MASTER_TIMEOUT_MS/ portTICK_PERIOD_MS);
    //ESP_LOGI("bus_read", "data: %x, count: %d", reg_data[0], cnt);
    return (int8_t)ret;
}

// int8_t BME280_I2C_bus_write( uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt, uint8_t dev_addr)
// {
//     ESP_LOGI("i2c bus write", "%d", dev_addr);
//     int32_t bme_error = 0;//BME280_INIT_VALUE;
//     esp_err_t err;
// 	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
// 	i2c_master_start(cmd);
// 	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);

// 	i2c_master_write_byte(cmd, reg_addr, true);
// 	i2c_master_write(cmd, reg_data, cnt, true);
// 	i2c_master_stop(cmd);

// 	err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 10/portTICK_PERIOD_MS);
// 	if (err == ESP_OK) {
// 		bme_error = 1;
// 	} else {
// 		bme_error = -1;
// 	}
// 	i2c_cmd_link_delete(cmd);

// 	return (int8_t)bme_error;
// }

// int8_t BME280_I2C_bus_read(uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt, uint8_t dev_addr)
// {
//     int32_t bme_error = 0;//BME280_INIT_VALUE;
// 	esp_err_t err;
//     ESP_LOGI("i2c bus read_reg_addr", "%x", reg_addr);
//     ESP_LOGI("i2c bus read_cnt", "%x", cnt);
//     //ESP_LOGI("i2c bus read_dev_addr", "%x", *(uint8_t*)dev_addr);

// 	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

// 	i2c_master_start(cmd);
// 	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
// 	i2c_master_write_byte(cmd, reg_addr, true);

// 	i2c_master_start(cmd);
// 	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

// 	if (cnt > 1) {
// 		i2c_master_read(cmd, reg_data, cnt-1, I2C_MASTER_ACK);
// 	}
//     ESP_LOGI("i2c bus regdata", "%x", *reg_data);
// 	i2c_master_read_byte(cmd, reg_data+cnt-1, I2C_MASTER_NACK);
// 	i2c_master_stop(cmd);

// 	err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 10/portTICK_PERIOD_MS);
// 	if (err == ESP_OK) {
// 		bme_error = 1;
// 	} else {
// 		bme_error = -1;
// 	}

// 	i2c_cmd_link_delete(cmd);

// 	return (int8_t)bme_error;
// }

