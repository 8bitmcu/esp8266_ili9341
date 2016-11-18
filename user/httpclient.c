#include "httpclient.h"






void httpclient_readall(char **pdata, struct httpreq* req) {

    *pdata = (char *)os_zalloc(req->p->tot_len + 1);
    pbuf_copy_partial(req->p, *pdata, req->p->tot_len, 0);

    pbuf_free(req->p);
    tcp_close(req->pcb);
}

uint32_t httpclient_read(char *pdata, struct httpreq* req, uint32_t size) {

    // what's left to be read
    uint32_t data_left = req->res_size - req->reader;

    // if trying to read more than what's available
    if(size > data_left) 
        size = data_left;

    // if nothing left
    if(!size) {
        pbuf_free(req->p);
        tcp_close(req->pcb);
        return 0;
    }

    os_memcpy(pdata, (char *) req->p->payload + req->reader, size);
    
    pdata[size] = '\0';

    // increment reader by amount read
    req->reader += size;

    return size;
}





bool httpclient_request(struct httpreq* req) {

    os_printf("Request to %d.%d.%d.%d:%d\n", req->ip[0], req->ip[1], req->ip[2], req->ip[3], req->port);


    ip_addr_t addr;
    IP4_ADDR(&addr, req->ip[0], req->ip[1], req->ip[2], req->ip[3]);

    struct tcp_pcb* pcb = tcp_new();
    if (!pcb) {
        os_printf("pcb is NULL");
        return 0;
    }

    tcp_arg(pcb, req);
    tcp_err(pcb, httpclient_err);

    tcp_bind(pcb, IP_ADDR_ANY, req->port);
    tcp_connect(pcb, &addr, req->port, httpclient_connected);
}








static err_t ICACHE_FLASH_ATTR
httpclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {


    if (err != ERR_OK) {
        httpclient_err(tpcb, err);
        return err;
    }


    os_printf("Connected\n");
    char *string = "GET / HTTP/1.1\r\nHost: 192.168.0.11\r\n\r\n";


    //tcp_sent(tpcb, httpclient_sent);
    tcp_recv(tpcb, httpclient_recv);


    /*Disable Nagle algorithm default*/
    tcp_nagle_disable(tpcb);


    err = tcp_write(tpcb, string, strlen(string), 0);
    if(err) {
        httpclient_err(tpcb, err);
        return err;
    }


    err = tcp_output(tpcb);
    if(err) {
        httpclient_err(tpcb, err);
        return err;
    }

    return err;
}


static void ICACHE_FLASH_ATTR
httpclient_err(void *arg, err_t err) {

    struct tcp_pcb* pcb = arg;

    os_printf("lwip error (%s)\n",lwip_strerr(err));
    os_printf("httpclient_err %d %d %d\n", pcb->state, pcb->nrtx, err);
}


/*
static err_t ICACHE_FLASH_ATTR
httpclient_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    os_printf("sent");
}
*/




static err_t ICACHE_FLASH_ATTR
httpclient_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {

    struct httpreq* req = arg;

    if(!req->res) {
        os_printf("Got response\n");

        req->res = true;
        req->pcb = pcb;
        req->p = p;

        // response size
        req->res_size = req->p->tot_len;

        req->res_cb(arg);
    }
}