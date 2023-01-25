#define SDA_PIN GPIO_NUM_15
#define SCL_PIN GPIO_NUM_2
#define I2C_MASTER_ACK 0
#define I2C_MASTER_NACK 1

#define I2C_MASTER_NUM 0


/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "192.168.0.103"
#define WEB_PORT "5000"
#define WEB_PATH "/"


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
