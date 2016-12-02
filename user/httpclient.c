#include "httpclient.h"




// send a (GET) request to a server specified by strct httpreq
err_t httpclient_request(struct httpreq* req) {

    err_t err;

    //os_printf("Request to %d.%d.%d.%d:%d\n", req->ip[0], req->ip[1], req->ip[2], req->ip[3], req->port);

    // create new protocol control block
    struct tcp_pcb* pcb = tcp_new();

    if (!pcb) {
        httpclient_tcp_err(pcb, ERR_PCBNULL);
        return err;
    }


    tcp_arg(pcb, req);
    tcp_err(pcb, httpclient_tcp_err);


    if(err) {
        httpclient_tcp_err(pcb, err);
        return err;
    }


    // create connection
    ip_addr_t addr;
    IP4_ADDR(&addr, req->ip[0], req->ip[1], req->ip[2], req->ip[3]);

    err = tcp_connect(pcb, &addr, req->port, httpclient_tcp_connected);

    if(err) {
        httpclient_tcp_err(pcb, err);
        return err;
    }


    return err;
}

// converts an error code to a readable char*
const char* http_errstr(err_t err) {
  return httperr[-err];
}




// tcp connection to server callback
static err_t ICACHE_FLASH_ATTR
httpclient_tcp_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {

    struct httpreq* req = arg;

    if (err) {
        httpclient_tcp_err(tpcb, err);
        return err;
    }

    // needs roughly ~40 char + path
    char string[64 + strlen(req->path)];

    // prepare http request header
    os_sprintf(
        string,
        "GET %s HTTP/1.1\r\nHost: %d.%d.%d.%d\r\n\r\n", 
        req->path,
        req->ip[0], 
        req->ip[1], 
        req->ip[2], 
        req->ip[3]
    );


    tcp_recv(tpcb, httpclient_tcp_recv);

    // disable Nagle algorithm
    tcp_nagle_disable(tpcb);


    err = tcp_write(tpcb, string, strlen(string), 0);
    if(err) {
        httpclient_tcp_err(tpcb, err);
        return err;
    }


    err = tcp_output(tpcb);
    if(err) {
        httpclient_tcp_err(tpcb, err);
        return err;
    }

    return err;
}

// tcp error callback
static void ICACHE_FLASH_ATTR
httpclient_tcp_err(void *arg, err_t err) {

    struct httpreq* req = arg;

    if(req->err_cb != NULL) {
        req->err_cb(err);
    }
}



// tcp packet received callback
static err_t ICACHE_FLASH_ATTR
httpclient_tcp_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {

    if(err) {
        httpclient_tcp_err(pcb, err);
        return err;
    }

    struct httpreq* req = arg;

    // p not NULL, confirm received
    if (p != NULL) {
        tcp_recved(pcb, p->len);
    }
    else {
        // p is NULL, close tcp connection
        err_t err = tcp_close(pcb);

        if(err) {
            httpclient_tcp_err(pcb, err);
        }

        return err;
    }


    // raise error when no response callback is set
    if(req->res_cb == NULL) {
        httpclient_tcp_err(pcb, ERR_RESCB);
        return err;
    }


    // read data from packet
    char *data = NULL;
    uint16_t len = 0;


    data = (char *)os_zalloc(p->len + 1);
    len = pbuf_copy_partial(p, data, p->len, 0);
    pbuf_free(p);

    // send data to response cb
    if (len) {
        req->res_cb(req, data, len);
    }
    os_free(data);


    return err;
}
