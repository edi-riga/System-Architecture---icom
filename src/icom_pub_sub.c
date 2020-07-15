#include <stdlib.h>
#include <zmq.h>

#include "icom.h"
#include "icom_common.h"
#include "string_parser.h"
#include "notifying.h"


icomPacket_t *icom_doPubDeep(icom_t *icom){
    int ret;
    void *socket;

    /* send packet sequence */
    for(int i=0; i<icom->socketCount; i++){
        socket = icom->sockets[i].socket;
        icomPacket_t *packet = &(icom->packets[icom->packetIndex]);

        /* send header */
        do{
            ret = zmq_send(socket, &(packet->header), 
                sizeof(icomPacketHeader_t), ZMQ_SNDMORE);
        } while( (ret == -1) && (errno == EAGAIN));

        /* send payload */
        do{
            _D("Sending payload");
            ret = zmq_send(socket, packet->payload, packet->header.size, 0);
        } while( (ret == -1) && (errno == EAGAIN));
    }

    /* update buffer index */
    if(++icom->packetIndex >= icom->packetCount)
        icom->packetIndex = 0;
    
    return &(icom->packets[icom->packetIndex]);
}

icomPacket_t *icom_doSubDeep(icom_t *icom){
    int ret;
    void *socket;

    /* receive all buffers from all sockets */
    for(int i=0; i<icom->socketCount; i++){
        socket = icom->sockets[i].socket;
        icomPacket_t *packet = &(icom->packets[icom->packetIndex]);

        /* receive header */
        do{
            ret = zmq_recv(socket, &(packet->header), 
              sizeof(icomPacketHeader_t),ZMQ_RCVMORE);
        } while( (ret == -1) && (errno == EAGAIN));

        
        /* receive payload */
        do{
            ret = zmq_recv(socket, packet->payload, packet->header.size, 0);
        } while( (ret == -1) && (errno == EAGAIN));
    }

    return icom->packets;
}


int icom_initPubSockets(icom_t *icom){
    icom->sockets = (icomSocket_t*)malloc(icom->socketCount*sizeof(icomSocket_t));

    for(int i=0; i<icom->socketCount; i++){
        icomSocket_t *iSocket = &(icom->sockets[i]);
        iSocket->string = icom->comStrings[i];
        iSocket->inproc = (strstr("inproc", icom->comStrings[i]) != NULL)? 1 : 0;

        iSocket->socket = zmq_socket(getContext(), ZMQ_PUB);
        if(iSocket->socket == NULL){
            _SE("Failed to create ZMQ socket");
            return -1;
        }

        if( zmq_bind(iSocket->socket, iSocket->string) ){
            _SE("Failed to bind ZMQ socket to \"%s\" string", iSocket->string);
            return -1;
        }
    }

    return 0;
}


int icom_initSubSockets(icom_t *icom){
    icom->sockets = (icomSocket_t*)malloc(icom->socketCount*sizeof(icomSocket_t));

    for(int i=0; i<icom->socketCount; i++){
        icomSocket_t *iSocket = &(icom->sockets[i]);
        iSocket->string = icom->comStrings[i];
        iSocket->inproc = (strstr("inproc", icom->comStrings[i]) != NULL)? 1 : 0;

        iSocket->socket = zmq_socket(getContext(), ZMQ_SUB);
        if(iSocket->socket == NULL){
            _SE("Failed to create ZMQ socket");
            return -1;
        }

        if( zmq_connect(iSocket->socket, iSocket->string) ){
            _SE("Failed to connect ZMQ socket to \"%s\" string", iSocket->string);
            return -1;
        }

        // ignore ZMQ topic filtering feature
        zmq_setsockopt(iSocket->socket, ZMQ_SUBSCRIBE, "", 0);
    }

    return 0;
}



icom_t *icom_initPublish(
char      *comString,
unsigned   payloadSize,
unsigned   packetCount,
uint32_t   flags)
{
    /* allocate and initialize icom structure */
    icom_t *icom      = (icom_t*)malloc(sizeof(icom_t));
    icom->packetCount = packetCount;
    icom->flags       = flags;
    icom->type        = ICOM_TYPE_PUB;
    icom->packetIndex = 0;

    /* check if zero copy is asked for */
    if(flags & ICOM_ZERO_COPY & ICOM_PROTECTED){
        _W("Sorry, zero copy + protected feature has not been implemented");
    } else if (flags & ICOM_ZERO_COPY){
        _W("Sorry, zero copy feature has not been implemented");
    } else {
        icom->cbDo     = icom_doPubDeep; 
    }

    /* initialize communication strings (each strings corresponds to a socket) */
    int ret = parser_initStrArray(&(icom->comStrings), &(icom->socketCount), comString);
    if(ret != 0){
        _E("Failed to parse communication string");
        return NULL;
    }

    /* the initialize communication strings */
    icom->packetCount = icom->socketCount;

    /* initialize sockets */
    ret = icom_initPubSockets(icom);
    if(ret != 0){
        _E("Failed to initialize PUB sockets");
        return NULL;
    }

    /* initialize packets */
    ret = icom_initPackets(icom, payloadSize);
    if(ret != 0){
        _E("Failed to initialize packets");
        return NULL;
    }

    return icom;
}


icom_t *icom_initSubscribe(
char     *comString,
unsigned  payloadSize,
uint32_t  flags)
{
    /* allocate and initialize icom structure */
    icom_t *icom = (icom_t*)malloc(sizeof(icom_t));
    icom->flags  = flags;
    icom->type   = ICOM_TYPE_SUB;
    icom->packetIndex = 0;

    /* setup communication callback */
    if(flags & ICOM_ZERO_COPY & ICOM_PROTECTED){
        _W("Sorry, zero copy + protected feature has not been implemented");
    } else if (flags & ICOM_ZERO_COPY){
        _W("Sorry, zero copy feature has not been implemented");
    } else {
        icom->cbDo     = icom_doSubDeep; 
    }

    /* initialize communication strings (each strings corresponds to a socket) */
    int ret = parser_initStrArray(&(icom->comStrings), &(icom->socketCount), comString);
    if(ret != 0){
        _E("Failed to parse communication string");
        return NULL;
    }

    /* the initialize communication strings */
    icom->packetCount = icom->socketCount;

    /* initialize sockets */
    ret = icom_initSubSockets(icom);
    if(ret != 0){
        _E("Failed to initialize PUB sockets");
        return NULL;
    }

    /* initialize packets */
    ret = icom_initPackets(icom, payloadSize);
    if(ret != 0){
        _E("Failed to initialize packets");
        return NULL;
    }

    return icom;
}
