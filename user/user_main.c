#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "uart.h"


#define LWIP_OPEN_SRC

#include "user_interface.h"
#include "httpclient.h"




#define user_procTaskPrio        0
#define user_procTaskQueueLen    1


os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;



void some_timerfunc(void *arg) {

    // connect to client
    uint8_t ip[4] = {192, 168, 0, 11};
    httpclient_connect(ip, 80);



    os_delay_us(100*1000);
}


//Do nothing function
static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events) {
    os_delay_us(10);
}

//Init function
void ICACHE_FLASH_ATTR user_init() {

    // Initialize the GPIO subsystem.
    gpio_init();

    uart_init(BIT_RATE_9600, BIT_RATE_9600);




    // init WIFI

 
    struct station_config stationConf; 
 
    wifi_set_opmode( 0x01 ); 
    os_memcpy(&stationConf.ssid, SSID_NAME, 32); 
    os_memcpy(&stationConf.password, SSID_PWD, 32); 
    wifi_station_set_config(&stationConf); 
    wifi_station_connect(); 




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
