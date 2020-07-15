#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <zmq.h>

#include "icom.h"
#include "icom_common.h"
#include "notifying.h"

/*************** Globals ***************/
static void *zmqContext = NULL; // single context per application


void *getContext(){
    static pthread_mutex_t lockGetContext;

    pthread_mutex_lock(&lockGetContext);

    if(zmqContext == NULL){
        zmqContext = zmq_ctx_new();

        if(zmqContext == NULL)
            _SE("Failed to initialize ZMQ context");
    }

    pthread_mutex_unlock(&lockGetContext);

    return zmqContext;
}

void setSockopt(void *socket, int name, int value){
    if(zmq_setsockopt(socket, name, &value, sizeof(value)) == -1)
        _SW("Failed to set socket option");
}


int icom_initPackets(icom_t *icom, unsigned payloadSize){
    icom->packets = (icomPacket_t*)malloc(icom->packetCount*sizeof(icomPacket_t));

    _D("Allocated packets: %d", hasAllocatedBuffers(icom));
    if(hasAllocatedBuffers(icom)){
        _D("Initializing packets");
        for(int i=0; i<icom->packetCount; i++){
            //(*packets)[i].header.type = ; TODO
            (icom->packets)[i].header.size = payloadSize;
            (icom->packets)[i].payload     = malloc(payloadSize);
        }
    } else {
        for(int i=0; i<icom->packetCount; i++)
            (icom->packets)[i].payload = NULL;
    }

    if( shouldInitializeSemaphores(icom)){
        _D("Initializing semaphores");
        int ret;
        for(int i=0; i<icom->packetCount; i++){
            ret = sem_init(&icom->packets[i].semWrite, 0, 1);
            if(ret == -1)
                _SW("Failed to initialize semaphore");

            ret = sem_init(&icom->packets[i].semRead,  0, icom->socketCount);
            if(ret == -1)
                _SW("Failed to initialize semaphore");
        }
    }

    _D("Linking the list");
    for(int i=0; i<icom->packetCount-1; i++){
        (icom->packets)[i].next = &((icom->packets)[i+1]);
    }
    (icom->packets)[icom->packetCount-1].next = NULL;

    return 0;
}
