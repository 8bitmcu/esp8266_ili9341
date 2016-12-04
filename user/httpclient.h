#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H


#define LWIP_OPEN_SRC
//#define LWIP_DEBUG

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
#include "lwip/err.h"



// Copied from lwip/err.c
// Didn't seem to be accessible even with LWIP_DEBUG enabled
// (maybe because it's linked instead of compiled?)

// defining my own errors
#define ERR_PCBNULL -16
#define ERR_RESCB -17


static const char *httperr[] = {
	"Ok.",                    /* ERR_OK          0  */
	"Out of memory error.",   /* ERR_MEM        -1  */
	"Buffer error.",          /* ERR_BUF        -2  */
	"Timeout.",               /* ERR_TIMEOUT    -3  */
	"Routing problem.",       /* ERR_RTE        -4  */
	"Operation in progress.", /* ERR_INPROGRESS -5  */
	"Illegal value.",         /* ERR_VAL        -6  */
	"Operation would block.", /* ERR_WOULDBLOCK -7  */
	"Connection aborted.",    /* ERR_ABRT       -8  */
	"Connection reset.",      /* ERR_RST        -9  */
	"Connection closed.",     /* ERR_CLSD       -10 */
	"Not connected.",         /* ERR_CONN       -11 */
	"Illegal argument.",      /* ERR_ARG        -12 */
	"Address in use.",        /* ERR_USE        -13 */
	"Low-level netif error.", /* ERR_IF         -14 */
	"Already connected.",     /* ERR_ISCONN     -15 */

	"pcb is NULL.",
	"httpreq->res_cb is NULL.",
};






struct httpreq;


// callback prototypes
typedef void (* err_callback)(err_t err);
typedef void (* res_callback)(struct httpreq* req, char* data, uint16_t len);


struct httpreq {

	// Remote ip
	uint8_t ip[4];

	// remote port
	uint16_t port;

	// path to query
	char* path;


	// fn to get called when a response is received
    res_callback res_cb;

    // fn to get called when an error occur
    err_callback err_cb;
};


// send a (GET) request to a server specified by strct httpreq
err_t httpclient_request(struct httpreq* req);


// converts an error code to a readable char*
const char* http_errstr(err_t err);


// tcp connection to server callback
static err_t ICACHE_FLASH_ATTR
httpclient_tcp_connected(void *arg, struct tcp_pcb *tpcb, err_t err);


// tcp error callback
static void ICACHE_FLASH_ATTR
httpclient_tcp_err(void *arg, err_t err);


// tcp packet received callback
static err_t ICACHE_FLASH_ATTR
httpclient_tcp_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);






char* ICACHE_FLASH_ATTR
httpclient_getbodyptr(char* str);


#endif