#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "uart.h"

#include "spi.h"
#include "ili9341.h"
#include "httpclient.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;



//uint16_t color = ILI9340_GREEN;

void some_timerfunc(void *arg) {

  //color = color == ILI9340_GREEN ? ILI9340_BLUE : ILI9340_GREEN;


    //fillScreen(color);

    os_printf("Trying wifi...");

    struct station_config stationConf;

    wifi_set_opmode( STATION_MODE );
    os_memcpy(&stationConf.ssid, SSID_NAME, 32);
    os_memcpy(&stationConf.password, SSID_PWD, 32);
    wifi_station_set_config(&stationConf);
    wifi_station_connect();


    http_get("http://api.ipify.org/?format=text", "", http_callback_example);


    os_delay_us(100*1000);
}


/*
void http_callback_example(char * response_body, int http_status, char * response_headers, int body_size)
{
    os_printf("http_status=%d\n", http_status);
    if (http_status != HTTP_STATUS_GENERIC_ERROR) {
        os_printf("strlen(headers)=%d\n", strlen(response_headers));
        os_printf("body_size=%d\n", body_size);
        os_printf("body=%s<EOF>\n", response_body);
    }
}
*/


//Do nothing function
static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events) {
    os_delay_us(10);
}

//Init function
void ICACHE_FLASH_ATTR user_init() {

    // Initialize the GPIO subsystem.
    gpio_init();

    uart_init(BIT_RATE_9600, BIT_RATE_9600);

    ili_init(true);

    //Disarm timer
    os_timer_disarm(&some_timer);

    //Setup timer
    os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

    //Arm the timer
    //&some_timer is the pointer
    //1000 is the fire time in ms
    //0 for once and 1 for repeating
    os_timer_arm(&some_timer, 10000, 1);

    //Start os task
    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}
