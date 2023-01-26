/*
* Platform specific implementations of functions required by generic Bosche BME280 driver
* Author: Matthew Smith, github: @Paumanok
*/
void BME280_delay_us(uint32_t usec);

int8_t BME280_I2C_bus_write( uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt, uint8_t dev_addr);

int8_t BME280_I2C_bus_read(uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt, uint8_t dev_addr);