#include "httpclient.h"



// read the whole response into *data
void httpclient_readall(char **data, struct httpreq* req) {

    // allocate memory for the response
    *data = (char *)os_zalloc(req->p->tot_len + 1);
    
    // copy & free buffer
    pbuf_copy_partial(req->p, *data, req->p->tot_len, 0);
    pbuf_free(req->p);

    // close tcp connection
    err_t err = tcp_close(req->pcb);

    if(err) {
        httpclient_tcp_err(req->pcb, err);
    }

    os_free(req);
}

// read an amount of bytes into *data specified by size
uint32_t httpclient_read(char *data, struct httpreq* req, uint32_t size) {

    // what's left to be read
    uint32_t data_left = req->res_size - req->bytes_read;

    // if trying to read more than what's available
    if(size > data_left) 
        size = data_left;

    // if nothing left, close connection
    if(!size) {
        pbuf_free(req->p);
        err_t err = tcp_close(req->pcb);

        if(err) {
            httpclient_tcp_err(req->pcb, err);
        }
        return 0;

        os_free(req);
    }

    os_memcpy(data, (char *) req->p->payload + req->bytes_read, size);

    data[size] = '\0';

    // increment bytes_read by amount read
    req->bytes_read += size;

    return size;
}

// send a (GET) request to a server specified by strct httpreq
err_t httpclient_request(struct httpreq* req) {

    err_t err;

    //os_printf("Request to %d.%d.%d.%d:%d\n", req->ip[0], req->ip[1], req->ip[2], req->ip[3], req->port);

    // create new protocol control block
    struct tcp_pcb* pcb = tcp_new();

    if (!pcb) {
        httpclient_tcp_err(pcb, -16);
    }


    tcp_arg(pcb, req);
    tcp_err(pcb, httpclient_tcp_err);


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


    /*Disable Nagle algorithm default*/
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

/*
    os_printf("httpclient_tcp_err\nlwip tcp error: %s\n",tcperr_strerr[-err]);

    if(req->pcb) {
        os_printf("pcb state: %d, nrtx: %d\n", req->pcb->state, req->pcb->nrtx);
    }*/
}


// tcp packet received callback
static err_t ICACHE_FLASH_ATTR
httpclient_tcp_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {

    if(err) {
        httpclient_tcp_err(pcb, err);
        return err;
    }

    struct httpreq* req = arg;

    if(!req->res) {
        req->res = true;
        req->pcb = pcb;
        req->p = p;

        // response size
        req->res_size = req->p->tot_len;

        req->res_cb(arg);
    }

    return err;
}
