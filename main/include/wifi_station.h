#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


/* FreeRTOS event group to signal when we are connected*/
extern EventGroupHandle_t s_wifi_event_group;

void wifi_init_sta(void);