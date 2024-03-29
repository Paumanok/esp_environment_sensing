/*
* main.c
* esp32/s2 environment sensing for use with a self hosted REST server for tracking data. 
* temp/humidity/pressure measured using a Bosch BME280
* pm25 measured using an ikea vindriktning
* Author: Matthew smith, Github: @paumanok
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
//#include "protocol_examples_common.h" //solely there for example_connect() that does fancy error checking
#include "driver/i2c.h"
#include "driver/uart.h"
#include "cJSON.h"
#include "sdkconfig.h"
#include "esp_sleep.h"

//my little guys//
#include "request.h"
#include "wifi_station.h"
#include "common.h"

#include "bme280.h"
#include "bme280_user.h"
//end little guys

//Globals
struct bme280_dev dev;
pm25_state pm25;

uint8_t dev_mac[6];
char mac_str[MAC_STR_LEN];

const uart_port_t uart_num = USER_UART_NUM;
//end Globals



static void bme280_measure_task(void* pvParameters)
{
    struct bme280_data data; 

    char post_req[512];
    char post_json[255];
    char post_clen[32];
    char* post_json_fmt = "\r\n{\"mac\": \"%s\", \"temp\": %.2f, \"hum\": %.2f}\r\n\r\n";
    int json_len;

    while(1)
    {
        dev.delay_us(7000, dev.intf_ptr);
        bme280_get_sensor_data(BME280_ALL, &data, &dev);
        ESP_LOGI("TAG_BME280", "%.2f degF / %.3f hPa / %.3f %%", (data.temperature*(9/5))+32, data.pressure/100, data.humidity);

        sprintf(post_json, post_json_fmt, mac_str, data.temperature, data.humidity, data.pressure);
        json_len = strlen(post_json);
        
        sprintf(post_clen, POST_CLEN, json_len-6);

        sprintf(post_req, "%s%s%s", POST_HEADER, post_clen, post_json);


        ESP_LOGI("measure_task","request: %s", post_req);
        
        http_post_single(post_req);
        int next_countdown = http_get_single();
        
        esp_sleep_enable_timer_wakeup(next_countdown * 1000000); //time is in microseconds
        esp_light_sleep_start();
        //for(int countdown = next_countdown; countdown >= 0; countdown--) {
        //    if(countdown % 10 == 0)
        //        ESP_LOGI("BME280_MEASUREMENT_TASK", "t-minus: %d... ", countdown);
        //    vTaskDelay(1000 / portTICK_PERIOD_MS);
        //}
    }
}

void get_bme280_measurements(struct bme280_data *data)
{
    dev.delay_us(7000, dev.intf_ptr);
    bme280_get_sensor_data(BME280_ALL, data, &dev);
    ESP_LOGI("TAG_BME280", "%.2f degF / %.3f hPa / %.3f %%", (data->temperature*(9/5))+32, data->pressure/100, data->humidity);
}


/*
* measurement_task
* "main" task to coordinate getting measurements from various sensors. 
*/
static void measurement_task(void* pvParameters)
{
    struct bme280_data data;
    char post_req[512];
    char post_json[255];
    char post_clen[32];
    int json_len;

    while(1)
    {
        get_bme280_measurements(&data);

        if(PM25_ENABLED)
        {
            while(pm25.lock)
                vTaskDelay(50/ portTICK_PERIOD_MS);
            sprintf(post_json, post_json_fmt, mac_str, data.temperature, data.humidity, data.pressure, pm25.last_measurement);
        }
        else 
        {
            sprintf(post_json, post_json_fmt, mac_str, data.temperature, data.humidity, data.pressure);
        }

        json_len = strlen(post_json);
        
        sprintf(post_clen, POST_CLEN, json_len-6);

        sprintf(post_req, "%s%s%s", POST_HEADER, post_clen, post_json);
        ESP_LOGI("measure_task","request: %s", post_req);
        http_get_single(); //this is here because it seems like the first socket reconnect fails
        http_post_single(post_req);

        int next_countdown = http_get_single();
        //esp_wifi_disconnect();
        esp_wifi_stop();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        ESP_LOGI("measure_task", "entering sleep: %d seconds", next_countdown);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        
        esp_sleep_enable_timer_wakeup(next_countdown * 1000000); //time is in microseconds
        esp_light_sleep_start();
        
        esp_wifi_start();
        
        xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, 500 / portTICK_PERIOD_MS);
        //ESP_LOGI("measure_task","exiting sleep after %d seconds", next_countdown);
        //for(int countdown = next_countdown; countdown >= 0; countdown--)
        //{
        //    if(countdown % 10 == 0)
        //        ESP_LOGI("BME280_MEASUREMENT_TASK", "t-minus: %d... ", countdown);
        //    vTaskDelay(1000 / portTICK_PERIOD_MS);
        //}
    }
    
}



/*
*   uart_read_task
*   Polls the uart rx buffer to check for the air quality sensor messages
*/
static void uart_read_task(void* pvParameters)
{
    uint8_t data[128];
    char* data_str[129];
    int length = 0;
    pm25.lock = 0;
    pm25.last_measurement = 0;
    int temp_pm25;

    while(1)
    {
        // Read data from UART.
        
        length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t*)&length));
        length = uart_read_bytes(uart_num, data, length, 100);
        if( length > 0)
        {
            temp_pm25 = (data[5] << 8) | data[6];
            if(temp_pm25 != 0) //a zero value will mess up a graph, not sure if I should keep this
            {
                //don't write to it if the measurement task is trying to read it
                while(pm25.lock)
                    vTaskDelay(50/ portTICK_PERIOD_MS);
                //lock for our uses
                pm25.lock = !pm25.lock;
                pm25.last_measurement = temp_pm25;
                //unlock
                pm25.lock = !pm25.lock;
                ESP_LOGI("uart_read_task","###############recv: %d", pm25.last_measurement);
            }
            
        }
        
        uart_flush(uart_num);
        vTaskDelay(PM25_UART_POLL_DELAY/ portTICK_PERIOD_MS);
    }

}

void i2c_master_init()
{
    ESP_LOGI("i2c config", "%d %d", I2C_SDA, I2C_SCL);
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 400000
	};
	i2c_param_config(I2C_MASTER_NUM, &i2c_config);
	i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0);
}

void uart_init()
{
    
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(uart_num, 2048, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

}

void BME280_init()
{
    //pulled from the bosch git example
    int8_t rslt = BME280_OK;
    uint8_t dev_addr = BME280_I2C_ADDR_PRIM;

    dev.intf_ptr = &dev_addr;
    dev.intf = BME280_I2C_INTF;
    dev.read = &BME280_I2C_bus_read;
    dev.write = &BME280_I2C_bus_write;
    dev.delay_us = BME280_delay_us;

    rslt = bme280_init(&dev);
    ESP_LOGI("bme init res", "%x", rslt);

    uint8_t settings_sel;
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
	dev.settings.osr_p = BME280_OVERSAMPLING_1X;
	dev.settings.osr_t = BME280_OVERSAMPLING_2X;
	dev.settings.filter = BME280_FILTER_COEFF_OFF;
	dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;

    bme280_set_sensor_settings(settings_sel, &dev);
	bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);
}

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    //ESP_ERROR_CHECK(example_connect());
    wifi_init_sta();
    esp_read_mac(dev_mac, ESP_MAC_WIFI_STA);
    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", dev_mac[0], dev_mac[1], 
            dev_mac[2], dev_mac[3], dev_mac[4], dev_mac[5]);


    if(PM25_ENABLED)
    {
        uart_init();
        xTaskCreate(&uart_read_task, "uart_read_task", 4096, NULL,5,NULL); 
    }

    i2c_master_init();
    BME280_init();
    //xTaskCreate(&bme280_measure_task, "bme280_measure_task", 4096, NULL, 5, NULL);
    xTaskCreate(&measurement_task, "measurement_task", 4096, NULL, 5, NULL);
    
}
