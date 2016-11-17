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



static err_t ICACHE_FLASH_ATTR
httpclient_sent(void *arg, struct tcp_pcb *pcb, u16_t len);


static err_t ICACHE_FLASH_ATTR
httpclient_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);


static err_t ICACHE_FLASH_ATTR
httpclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err);


static void ICACHE_FLASH_ATTR
httpclient_err(void *arg, err_t err);


bool httpclient_connect(uint8_t *ip, uint16_t port);

#endif