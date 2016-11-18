#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H


#define LWIP_OPEN_SRC

#include "user_interface.h"

#include "lwip/opt.h"
#include "lwip/ip.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"



//TODO: refactor
#define espconn_printf(fmt, args...) os_printf(fmt,## args)



struct httpreq;


/** A callback prototype to inform about events for a espconn */
typedef void (* res_callback)(struct httpreq* req);
typedef void (* err_callback)(err_t err);

struct httpreq {


	uint8_t ip[4];

	uint16_t port;

	// has received response yet?
	bool res; 

    res_callback res_cb;
    err_callback err_cb;



    //~~~~~~split to httpres ???~~~~~

	// response size
	uint32_t res_size;

	// response size left to be read
	uint32_t reader;


	struct tcp_pcb *pcb;
	struct pbuf *p;


};



void httpclient_readall(char **pdata, struct httpreq* req);

bool httpclient_request(struct httpreq* req);




static err_t ICACHE_FLASH_ATTR
httpclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err);

static void ICACHE_FLASH_ATTR
httpclient_err(void *arg, err_t err);

static err_t ICACHE_FLASH_ATTR
httpclient_sent(void *arg, struct tcp_pcb *pcb, u16_t len);

static err_t ICACHE_FLASH_ATTR
httpclient_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);






#endif