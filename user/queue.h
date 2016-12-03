#ifndef QUEUE_H
#define QUEUE_H


#include "lwip/mem.h"


// A simple FIFO queue that dequeues data as soon as it can,
// therefore only stores (a copy) of what it can't dequeue.

typedef void (* queue_dequeue)(char* data, uint16_t len);


struct queue_struct {

    // transaction size (how many bytes required to dequeue?)
    uint16_t tsize;

    // data queued (smaller than tsize)
    char *queued;

    // data queued length
    uint16_t queuedlen;

    // dequeue function that gets called when 
    // enqueue'd data length is bigger than tsize
    queue_dequeue dequeue;
};



void queue_enqueue(struct queue_struct *q, char *data, uint16_t len) {
    uint16_t written = 0;

    // dequeue anything left stored previously before dequeuing new data
    if(q->queuedlen > 0) {
        os_memcpy(q->queued + q->queuedlen, data, q->tsize - q->queuedlen);
        q->dequeue(q->queued, q->tsize);
        written = q->tsize - q->queuedlen;
    }

    // dequeue all new data received
    while(written <= len - q->tsize) {
        q->dequeue(data + written, q->tsize);
        written += q->tsize;
    }

    // store what's left
    q->queuedlen = len - written;

    if(q->queued == NULL) {
        q->queued = (char *) os_malloc(sizeof(char) * q->tsize);
    }
    os_memcpy(q->queued, data + written, q->queuedlen); 
}


#endif