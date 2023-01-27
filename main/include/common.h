/*
* common.h
* 
* various definitions for use between main.c, request.c and bme280_user.c
*
*
*/

#define I2C_MASTER_ACK 0
#define I2C_MASTER_NACK 1
#define I2C_MASTER_NUM 0
#define I2C_MASTER_TIMEOUT_MS 1000

#define MAC_STR_LEN 18

#define UART_BAUD_RATE 9600
#define PM25_UART_POLL_DELAY 2000

//#define BME280_FLOAT_ENABLE

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "192.168.0.103"
#define WEB_PORT "5000"
#define WEB_PATH "/"


#define PM25_ENABLED 1

#if CONFIG_IDF_TARGET_ESP32
    //i2c pins
    #define I2C_SDA 18
    #define I2C_SCL 19

    #define UART_RX 26
    #define USER_UART_NUM 2
    //uart pins
#elif CONFIG_IDF_TARGET_ESP32S2
    //i2c pins
    #define I2C_SDA 13
    #define I2C_SCL 12
    //uart pins
    #define UART_RX 16
    #define USER_UART_NUM 1
#endif

//struct to maintain lock on measurement to avoid issues
//this may not be needed, not sure if freertos will actually run two tasks truely parallel
typedef struct
{
    uint16_t last_measurement;
    int lock;
}pm25_state;


/*
* Format strings used for creating responses
*/
#if PM25_ENABLED
static const char *post_json_fmt = "\r\n{\"mac\": \"%s\", \"temp\": %.2f, \"hum\": %.2f, \"pressure\": %.2f, \"pm25\": %d}\r\n\r\n";
#else
static const char *post_json_fmt = "\r\n{\"mac\": \"%s\", \"temp\": %.2f, \"hum\": %.2f, \"pressure\": %.2f}\r\n\r\n";
#endif
static const char *TAG = "HT_ESP";


static const char *REQUEST_NEXT = "GET /next" " HTTP/1.1\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

static const char *POST_HEADER ="POST /data " " HTTP/1.1\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Accept: */*\r\n"
    "Content-Type: application/json\r\n";

static const char *POST_CLEN = "Content-Length: %d\r\n";
static const char *POST_JSON = "\r\n"
                                "%s\r\n"
                                "\r\n";


static const char *REQUEST_POST_MEASUREMENT = "POST /data " " HTTP/1.1\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Accept: */*\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 24\r\n"
    "\r\n"
    "{\"temp\": %d, \"hum\": %d}\r\n"
    "\r\n";
