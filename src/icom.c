#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <zmq.h>

#include "icom.h"
#include "icom_common.h"
#include "string_parser.h"
#include "notifying.h"


int icom_init(){
    getContext();  // initializes zmq context
    return 0;
}

void icom_release(){
    zmq_ctx_destroy(getContext());
}


/*============================================================================*/
/*                             ICOM COMMON - API                              */
/*============================================================================*/

void icom_deinit(icom_t *icom){
    /* release memory buffers, but and check  */
    if(hasAllocatedBuffers(icom)){
        for(int i=0; i<icom->packetCount; i++){
            free(icom->packets[i].payload);
        }
    }

    /* destroy semaphores if they should have been allocated */
    if(shouldInitializeSemaphores(icom)){
        _D("Destroying semaphores");
        for(int i=0; i<icom->packetCount; i++){
            sem_destroy(&icom->packets[i].semWrite);
            sem_destroy(&icom->packets[i].semRead);
        }
    }

    /* unbind sockets if necesarry */
    if(shouldUnbindSocket(icom)){
        for(int i=0; i<icom->socketCount; i++)
            zmq_unbind(icom->sockets[i].socket, icom->sockets[i].string);
    }

    /* closing all opened sockets */
    for(int i=0; i<icom->socketCount; i++)
        zmq_close(icom->sockets[i].socket);

    /* release memory and socket buffers */
    free(icom->packets);
    free(icom->sockets);

    /* release allocated strings */
    parser_deinitStrArray(icom->comStrings, icom->socketCount);

    /* release holding struct */
    free(icom);
}

/*============================================================================*/
/*                           ICOM COMMON - PACKETS                            */
/*============================================================================*/
int icom_sendPacket(icomSocket_t *socket, icomPacket_t *packet){
    _D("Sending header");
    zmq_send(socket->socket, &(packet->header), sizeof(icomPacketHeader_t), 0);

    _D("Sending payload");
    return zmq_send(socket->socket, packet->payload, packet->header.size, 0);
}

int icom_recvPacket(icomSocket_t *socket, icomPacket_t *packet){
    _D("Receiving header");
    zmq_recv(socket->socket, &(packet->header), sizeof(icomPacketHeader_t), 0);

    _D("Receiving payload");
    return zmq_recv(socket->socket, packet->payload, packet->header.size, 0);
}

// TODO: find more efficient
int icom_sendPacketZero(icomSocket_t *socket, icomPacket_t *packet){
    int size = 0;
    _D("Sending zero copy packet");

    _D("\"%s\": Sending reference to header", socket->string);
    size += zmq_send(socket->socket, &packet->header, sizeof(icomPacketHeader_t*), 0);

    _D("\"%s\": Sending reference to payload", socket->string);
    size += zmq_send(socket->socket, &packet->payload, sizeof(icomPacketPayload_t*), 0);

    _D("\"%s\": Sending reference to semWrite", socket->string);
    size += zmq_send(socket->socket, &packet->semWrite, sizeof(sem_t*), 0);

    _D("\"%s\": Sending reference to semRead", socket->string);
    size += zmq_send(socket->socket, &packet->semRead, sizeof(sem_t*), 0);

    return size;
    //return zmq_send(socket->socket, packet, sizeof(icomPacket_t), 0);
}

// TODO: find more efficient
int icom_recvPacketZero(icomSocket_t *socket, icomPacket_t *packet){
    int size = 0;

    _D("Receiving zero-copy packet");

    _D("\"%s\": Receiving reference to header", socket->string);
    zmq_recv(socket->socket, &packet->header, sizeof(icomPacketHeader_t*), 0);

    _D("\"%s\": Receiving reference to payload", socket->string);
    zmq_recv(socket->socket, &packet->payload, sizeof(icomPacketPayload_t*), 0);

    _D("\"%s\": Receiving reference to semWrite", socket->string);
    zmq_recv(socket->socket, &packet->semWrite, sizeof(sem_t*), 0);

    _D("\"%s\": Receiving reference to semRead", socket->string);
    zmq_recv(socket->socket, &packet->semRead, sizeof(sem_t*), 0);

    return size;
    //return zmq_recv(socket->socket, packet, sizeof(icomPacket_t), 0);
}



icomPacket_t *icom_getCurrentPacket(icom_t *icom){
    return &(icom->packets[icom->packetIndex]);
}

/******************************************************************************/
/******************************** PUSH SOCKETS ********************************/
/******************************************************************************/
icomPacket_t *icom_doPushDeep(icom_t *icom){
    /* send packet sequence */
    for(int i=0; i<icom->socketCount; i++)
        icom_sendPacket(&(icom->sockets[i]), &(icom->packets[icom->packetIndex]));

    /* update buffer index */
    if(++icom->packetIndex >= icom->packetCount)
        icom->packetIndex = 0;
    
    return &(icom->packets[icom->packetIndex]);
}

icomPacket_t *icom_doPushZero(icom_t *icom){
    /* send packet sequence */
    for(int i=0; i<icom->socketCount; i++)
        icom_sendPacketZero(&(icom->sockets[i]), &(icom->packets[icom->packetIndex]));

    /* update buffer index */
    if(++icom->packetIndex >= icom->packetCount)
        icom->packetIndex = 0;
    
    return &(icom->packets[icom->packetIndex]);
}

icomPacket_t *icom_doPushZeroProtected(icom_t *icom){
    /* acquire write semaphore */
    _D("Acquiring semaphore");
    sem_wait(&(icom->packets[icom->packetIndex].semWrite));
    
    
    /* wait for the posted read semaphores (TODO: avoid polling) */
    _D("Checking read semaphore value");
    int sem_value;
    do{
        sem_getvalue(&(icom->packets[icom->packetIndex].semRead), &sem_value);
        _D("sem_value: %d", sem_value);
    } while(sem_value != icom->socketCount);

    /* send packet sequence */
    _D("Sending packet sequence");
    for(int i=0; i<icom->socketCount; i++)
        icom_sendPacketZero(&(icom->sockets[i]), &(icom->packets[icom->packetIndex]));

    /* post write semaphore */
    _D("Posting semaphore");
    sem_post(&(icom->packets[icom->packetIndex].semWrite));
    
    /* update buffer index */
    _D("Updating packet index");
    if(++icom->packetIndex >= icom->packetCount)
        icom->packetIndex = 0;

    return &(icom->packets[icom->packetIndex]);
}


int icom_initPushSocket(icomSocket_t *socket, char *string){
    socket->string = string;
    socket->inproc = (strstr("inproc", string) != NULL)? 1 : 0;

    socket->socket = zmq_socket(getContext(), ZMQ_PUSH);
    if(socket->socket == NULL){
        _SE("Failed to create ZMQ socket");
        return -1;
    }

    /* this is the best we can do in terms of PUSH syncronization */
    setSockopt(socket->socket, ZMQ_SNDHWM, 1);

    if( zmq_connect(socket->socket, socket->string) ){
        _SE("Failed to connect ZMQ socket to \"%s\" string", socket->string);
        return -1;
    }

    return 0;
}

int icom_initPushSockets(icomSocket_t **sockets, unsigned socketCount, char** comStrings){
    *sockets = (icomSocket_t*)malloc(socketCount*sizeof(icomSocket_t));

    for(int i=0; i<socketCount; i++){
        if(icom_initPushSocket(*sockets+i, comStrings[i]) != 0 ){
            _E("Failed to initialize PULL socket");
            return -1;
        }
    }

    return 0;
}

icom_t *icom_initPush(char *comString, unsigned payloadSize, unsigned packetCount, uint32_t flags){
    /* allocate and initialize icom structure */
    icom_t *icom = (icom_t*)malloc(sizeof(icom_t));
    icom->packetCount = packetCount;
    icom->packetIndex = 0;
    icom->flags       = flags;
    icom->type        = ICOM_TYPE_PUSH;

    /* check if zero copy is asked for (TODO: PROTECTED) */
    if( flags & ICOM_ZERO_COPY){
        if( flags & ICOM_PROTECTED){
            icom->cbDo = icom_doPushZeroProtected;
        } else{
            icom->cbDo = icom_doPushZero;
        }
    } else {
        icom->cbDo = icom_doPushDeep;
    }

    /* parse communication strings */
    int ret = parser_initStrArray(&(icom->comStrings), &(icom->socketCount), comString);
    if(ret != 0){
        _E("Failed to parse communication string");
        return NULL;
    }

    ret = icom_initPushSockets(&(icom->sockets), icom->socketCount, icom->comStrings);
    if(ret != 0){
        _E("Failed to initialize PUSH sockets");
        return NULL;
    }

    ret = icom_initPackets(icom, payloadSize);
    if(ret != 0){
        _E("Failed to initialize packets");
        return NULL;
    }

    return icom;
}

void icom_deinitPush(icom_t *icom){
    /* release memory buffers */
    for(int i=0; i<icom->packetCount; i++)
        free(icom->packets[i].payload);

    /* closing all opened sockets */
    for(int i=0; i<icom->socketCount; i++)
        zmq_close(icom->sockets[i].socket);

    /* release memory and socket buffers */
    free(icom->packets);
    free(icom->sockets);

    /* release allocated strings */
    parser_deinitStrArray(icom->comStrings, icom->socketCount);

    /* release holding struct */
    free(icom);
}


/******************************************************************************/
/******************************** PULL SOCKETS ********************************/
/******************************************************************************/
icomPacket_t *icom_doPullDeep(icom_t *icom){
    /* receive all buffers from all sockets */
    for(int i=0; i<icom->socketCount; i++){
        icom_recvPacket(&(icom->sockets[i]), &(icom->packets[i]));
    }

    return icom->packets;
}


icomPacket_t *icom_doPullZero(icom_t *icom){
    /* receive all buffers from all sockets */
    for(int i=0; i<icom->socketCount; i++){
        icom_recvPacketZero(&(icom->sockets[i]), &(icom->packets[i]));
    }

    return icom->packets;
}

icomPacket_t *icom_doPullZeroProtected(icom_t *icom){
    /* receive all buffers from all sockets */
    for(int i=0; i<icom->socketCount; i++){
        if(icom->packets[i].payload != NULL){
            _D("Posting previous semaphore");
            sem_post(&(icom->packets[i].semRead));
        }

        _D("Receiving buffers");
        icom_recvPacketZero(&(icom->sockets[i]), &(icom->packets[i]));

        _D("Acquiring semaphore");
        sem_wait(&(icom->packets[i].semRead));
    }

    return icom->packets;
}


int icom_initPullSocket(icomSocket_t *socket, char *string){
    socket->string = string;
    socket->inproc = (strstr("inproc", string) != NULL)? 1 : 0;

    socket->socket = zmq_socket(getContext(), ZMQ_PULL);
    if(socket->socket == NULL){
        _SE("Failed to create ZMQ socket");
        return -1;
    }

    /* this is the best we can do in terms of PUSH syncronization */
    setSockopt(socket->socket, ZMQ_RCVHWM, 1);

    if( zmq_bind(socket->socket, socket->string) ){
        _SE("Failed to bind ZMQ socket to \"%s\" string", socket->string);
        return -1;
    }

    return 0;
}

int icom_initPullSockets(icomSocket_t **sockets, unsigned socketCount, char** comStrings){
    *sockets = (icomSocket_t*)malloc(socketCount*sizeof(icomSocket_t));

    for(int i=0; i<socketCount; i++){
        if(icom_initPullSocket(*sockets+i, comStrings[i]) != 0){
            _E("Failed to initialize PULL socket");
            return -1;
        }
    }

    return 0;
}

icom_t *icom_initPull(char *comString, unsigned payloadSize, uint32_t flags){
    /* allocate and initialize icom structure */
    icom_t *icom = (icom_t*)malloc(sizeof(icom_t));
    icom->flags  = flags;
    icom->type   = ICOM_TYPE_PULL;
    icom->packetIndex = 0;

    /* check if zero copy is asked for (TODO: PROTECTED) */
    if(flags & ICOM_ZERO_COPY){
        if(flags & ICOM_PROTECTED){
            icom->cbDo     = icom_doPullZeroProtected; 
        } else{
            icom->cbDo     = icom_doPullZero; 
        }
    } else {
        icom->cbDo     = icom_doPullDeep; 
    }

    int ret = parser_initStrArray(&(icom->comStrings), &(icom->socketCount), comString);
    if(ret != 0){
        _E("Failed to parse communication string");
        return NULL;
    }

    icom->packetCount = icom->socketCount;

    ret = icom_initPullSockets(&(icom->sockets), icom->socketCount, icom->comStrings);
    if(ret != 0){
        _E("Failed to initialize PULL sockets");
        return NULL;
    }

    ret = icom_initPackets(icom, payloadSize);
    if(ret != 0){
        _E("Failed to initialize packets");
        return NULL;
    }

    return icom;
}
