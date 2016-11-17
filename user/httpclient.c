#include "httpclient.h"



static err_t ICACHE_FLASH_ATTR
httpclient_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    os_printf("sent");
}





static err_t ICACHE_FLASH_ATTR
httpclient_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    os_printf("recv");


    // blocking
    /*
	char *pdata = NULL;
	u16_t length = 0;
	pdata = (char *)os_zalloc(p->tot_len + 1);
    length = pbuf_copy_partial(p, pdata, p ->tot_len, 0);
    pbuf_free(p);

    os_printf(pdata);

    tcp_close(pcb);
    */


    // async
	char *pdata = NULL;
	u16_t qp = (p->tot_len / 4);

	pdata = (char *)os_zalloc(qp + 1);


	os_memcpy(pdata, p->payload, qp);
    os_printf(pdata);


	os_memcpy(pdata, p->payload + qp, qp);
    os_printf(pdata);


	os_memcpy(pdata, p->payload + (qp*2), qp);
    os_printf(pdata);


	os_memcpy(pdata, p->payload + (qp*3), qp);
    os_printf(pdata);





    pbuf_free(p);
    tcp_close(pcb);
}




static err_t ICACHE_FLASH_ATTR
httpclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {

    char *string = "GET / HTTP/1.1\r\nHost: 192.168.0.11\r\n\r\n";

    espconn_printf("espconn_client_connect pcon %p tpcb %p\n", arg, tpcb);

    if (err != ERR_OK) {
        os_printf("err in host connected (%s)\n",lwip_strerr(err));
        return err;
    }

    tcp_sent(tpcb, httpclient_sent);
    tcp_recv(tpcb, httpclient_recv);


    /*Disable Nagle algorithm default*/
    tcp_nagle_disable(tpcb);


    err = tcp_write(tpcb, string, strlen(string), 0);

    if(err) {
        os_printf("err in tcp_write (%s)\n",lwip_strerr(err));
        return err;
    }


    err = tcp_output(tpcb);

    if(err) {
        os_printf("err in tcp_output (%s)\n",lwip_strerr(err));
        return err;
    }



    return err;
}


static void ICACHE_FLASH_ATTR
httpclient_err(void *arg, err_t err) {

    struct tcp_pcb* pcb = arg;

    espconn_printf("espconn_client_err %d %d %d\n", pcb->state, pcb->nrtx, err);
}


bool httpclient_connect(uint8_t *ip, uint16_t port) {


    os_printf("Connect...");

    ip_addr_t addr;
    IP4_ADDR(&addr, ip[0], ip[1], ip[2], ip[3]);

    struct tcp_pcb* pcb = tcp_new();
    struct netif* interface = ip_route(&addr);

    if (!interface) {
        os_printf("no route to host\r\n");
        return 0;
    }

    if (!pcb) {
        os_printf("pcb is NULL");
        return 0;
    }

    tcp_bind(pcb, IP_ADDR_ANY, port);

    tcp_arg(pcb, pcb);
    tcp_err(pcb, httpclient_err);

    tcp_connect(pcb, &addr, port, httpclient_connected);

}