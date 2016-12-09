#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "uart.h"

#include "spi.h" 
#include "ili9341.h" 

#define LWIP_OPEN_SRC
//#define LWIP_DEBUG

#include "user_interface.h"
#include "httpclient.h"
#include "cencode.h"
#include "cdecode.h"
#include "queue.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1


os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;


#define SPI_FLASH_OFFSET 0x40000

uint32_t spiflash_index = 0;

// draw image from ROM
void rom_draw() {
    const uint16_t spi_t = 64;
    uint32_t i = 0;
    char data[spi_t];

    while(i < spiflash_index) {
        spi_flash_read(SPI_FLASH_OFFSET + i, (uint32 *) data, spi_t);
        write8(data, spi_t);

        i += spi_t;
    }
}



void rom_dequeue(char *data, uint16_t len) {

    // erase sector before writing to it
    if(spiflash_index == 0 || spiflash_index % SPI_FLASH_SEC_SIZE == 0) {
        spi_flash_erase_sector((SPI_FLASH_OFFSET + spiflash_index) / SPI_FLASH_SEC_SIZE);
    }

    // write to spi flash
    spi_flash_write(SPI_FLASH_OFFSET + spiflash_index, (uint32 *) data, len);

    // increment index
    spiflash_index += len;
}


void uart_dequeue(char *data, uint16_t len) {
    uint16_t i = 0;
    while(i++ < len) {
        os_printf("%X ",data[i]);
    }
    os_printf("\n");
}


char *outdata;
struct queue_struct* b64queue;
void b64_dequeue(char *data, uint16_t len) {

    if(outdata == NULL) {
        // un-encoded needs 33% less data than b64 encoded
        outdata = (char *) os_malloc(sizeof(char) * (len / 4 * 3));
    }

    // decode chunck
    base64_decodestate _state;
    base64_init_decodestate(&_state);
    uint16_t outlen = base64_decode_block(data, len, outdata, &_state);

    // init queue once
    if(b64queue == NULL) {
        b64queue = (struct queue_struct *)os_zalloc(sizeof(struct queue_struct));
        b64queue->tsize = 64;
        //b64queue->dequeue = write8;
        b64queue->dequeue = rom_dequeue;
        //b64queue->dequeue = uart_dequeue;
    }

    // queue data, gets written to LCD
    queue_enqueue(b64queue, outdata, outlen);
}




bool headers = false;
struct queue_struct* pqueue;
void response_callback(struct httpreq* req, char* data, uint16_t len) {


    if(!headers) {
        // get a ptr to the response body,
        // essentially ignoring headers
        char *c = httpclient_getbodyptr(data);

        // skip first 8 bytes (data info)
        c += 8;

        // re-calculate data length without headers
        len = len - (c - data);
        data = c;

        // set lcd to write ram for whole screen
        setWrite();

        // init queue
        pqueue = (struct queue_struct *)os_zalloc(sizeof(struct queue_struct));
        pqueue->tsize = 256;
        pqueue->dequeue = b64_dequeue;

        headers = true;
    }

    // enqueue & dequeue data to be b64 decoded
    queue_enqueue(pqueue, data, len);
}




void errr_callback(err_t err) {
    os_printf("lwip error %s", http_errstr(err));
}



bool once = false;
void some_timerfunc(void *arg) {

    // request once, redraw forever
    if(!once) {
        once = true;

            // reset global variable
        headers = false;

        // create new httprequest
        struct httpreq* req = (struct httpreq *)os_zalloc(sizeof(struct httpreq));

        // set server ip 
        uint8_t ip[4] = {192, 168, 0, 11};
        os_memcpy(req->ip, ip, 4); 

        // set port
        req->port = 8088;

        // set path to query
        req->path = "/bitmap?source=imgur"; 
        //req->path = "/bitmap?source=local&url=blue.png";

        // set response callback
        req->res_cb = response_callback;

        // set error callback
        req->err_cb = errr_callback;

        // send the request
        httpclient_request(req);
    }
    else {
        fillScreen(ILI9340_GREEN);

        rom_draw();
    }


}



// do nothing function
static void ICACHE_FLASH_ATTR 
user_procTask(os_event_t *events) {
    os_delay_us(10);
}


// main user function
void ICACHE_FLASH_ATTR 
user_init() {

    // initialize the GPIO subsystem.
    gpio_init();

    // init ili LCD
    ili_init(false); 

    // set LCD upside down (easier to dev from my desk, anyway)
    spiCommandData(ILI9340_MADCTL, ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR, 8);

    // fill LCD blue, so I know it's working
    fillScreen(ILI9340_BLUE);

    //uart_init(BIT_RATE_9600, BIT_RATE_9600);


    // init WIFI
    struct station_config stationConf; 
    wifi_set_opmode( 0x01 ); 
    os_memcpy(&stationConf.ssid, SSID_NAME, 32); 
    os_memcpy(&stationConf.password, SSID_PWD, 32); 
    wifi_station_set_config(&stationConf); 
    wifi_station_connect(); 





    // disarm timer
    os_timer_disarm(&some_timer);
    // setup timer
    os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
    // arm the timer
    os_timer_arm(&some_timer, 10000, 1);


    // start os task
    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}
