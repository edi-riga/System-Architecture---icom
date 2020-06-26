#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <zmq.h>
#include "icom.h"

/* INFO */
#define _I(fmt,args...)    printf(fmt "\n", ##args)
/* ERROR */
#define _E(fmt,args...)    printf("ERROR: " fmt "\n", ##args)
/* WARNING */
#define _W(fmt,args...)    printf("WARNING: " fmt "\n", ##args)
/* SYSTEM ERROR */
#define _SE(fmt,args...)   printf("SYSTEM ERROR (%s): " fmt "\n", strerror(errno), ##args)
/* SYSTEM WARNING  */
#define _SW(fmt,args...)   printf("SYSTEM WARNING (%s): " fmt "\n", strerror(errno), ##args)

/* DEBUGGING */
#if DEBUG
    #define _D(fmt,args...)  printf("DEBUG: "fmt "\n", ##args); fflush(stdout)
#else
    #define _D(fmt,args...)    
#endif


/*************** Globals ***************/
static void *zmqContext = NULL;        // single context per application
static pthread_mutex_t lockGetContext; // used for thread save initialization


/*************** Internal data structures ***************/
enum {
    ICOM_TYPE_PUSH=0,
    ICOM_TYPE_PULL,
    ICOM_TYPE_PUB,
    ICOM_TYPE_SUB,
};


/*************** Communication string parser API ***************/
#ifndef _GNU_SOURCE
static inline char* strchrnul(char *ptr, char delimiter){
    while(*ptr != delimiter && *ptr != '\0') ptr++;
    return ptr;
}
#endif

unsigned comStringGetCount(char *ptrStart){
    char *ptrStop;
    int   strCount = 0;
    int   idFrom, idTo, ret;

    do{
        // parse entry
        ptrStop = strchrnul(ptrStart, ',');
        char *tmp = strndup(ptrStart, ptrStop - ptrStart);
        ret = sscanf(tmp, "%*[^[][%d-%d]", &idFrom, &idTo);
        free(tmp);
       
        // update count 
        if(ret == 2)
            strCount += idTo - idFrom;
        strCount++;

        // set up for next iteration
        ptrStart = ptrStop + 1;
    }while(*ptrStop != '\0');
    
    return strCount;
}

int comStringGetArray(char ***strArray, unsigned *strCount, char *ptrStart){
    char *ptrStop, *idStr;
    int   idFrom, idTo, ret;
    unsigned strCurrent = 0;

    // allocate memories
    *strCount = comStringGetCount(ptrStart);
    *strArray = (char**)malloc((*strCount)*sizeof(char*));

    do{
        // parse entry
        ptrStop = strchrnul(ptrStart, ',');
        char *candidate = strndup(ptrStart, ptrStop - ptrStart);
        ret = sscanf(candidate, "%m[^[][%d-%d]", &idStr, &idFrom, &idTo);
       
        // update count 
        if(ret != 3){
            (*strArray)[strCurrent] = candidate;
            strCurrent++;
        }else{
            for(int i = idFrom; i <= idTo; i++){
                unsigned size = snprintf(NULL, 0, "%s%d", idStr, i) + 1;
                (*strArray)[strCurrent] = (char*)malloc(size*sizeof(char));
                sprintf((*strArray)[strCurrent], "%s%d", idStr, i);
                strCurrent++;
            }
            free(candidate);
        }
        free(idStr);

        // set up for next iteration
        ptrStart = ptrStop + 1;
    }while(*ptrStop != '\0');

    return 0;
}

void comStringClean(char ***strArray, unsigned strCount){
    for(int i=0; i<strCount; i++)
        free((*strArray)[i]);
    free(*strArray);
}


/*************** ICOM ZMQ-based API ***************/
/* Get (and create) ZMQ context */
static void *icom_getContext(){

    pthread_mutex_lock(&lockGetContext);

    if(zmqContext == NULL){
        zmqContext = zmq_ctx_new();

        if(zmqContext == NULL)
            _SE("Failed to initialize ZMQ context");
    }

    pthread_mutex_unlock(&lockGetContext);

    return zmqContext;
}

void icom_init(){
}

void icom_release(){
    zmq_ctx_destroy(zmqContext);
}

void icom_setSockopt(void *socket, int name, int value){
    if(zmq_setsockopt(socket, name, &value, sizeof(value)) == -1)
        _SW("Failed to set socket option");
}

int icom_updateConnectSockopt(void *socket, char *string, int name, int value){
    if(zmq_disconnect(socket, string) != 0){
        _SE("Failed to disconnect socket");
        return errno;
    }

    icom_setSockopt(socket, name, value);
    
    if(zmq_connect(socket, string) != 0){
        _SE("Failed to connect socket");
        return errno;
    };

    return 0;
}

int icom_updateBindSockopt(void *socket, char *string, int name, int value){
    if(zmq_unbind(socket, string) != 0){
        _SE("Failed to unbind socket");
        return errno;
    }

    icom_setSockopt(socket, name, value);
    
    if(zmq_unbind(socket, string) != 0){
        _SE("Failed to unbind socket");
        return errno;
    };

    return 0;
}

int icom_initPushSocket(icomSocket_t *socket, char *string){
    socket->string = string;
    socket->inproc = (strstr("inproc", string) != NULL)? 1 : 0;

    socket->socket = zmq_socket(icom_getContext(), ZMQ_PUSH);
    if(socket->socket == NULL){
        _SE("Failed to create ZMQ socket");
        return -1;
    }

    /* this is the best we can do in terms of PUSH syncronization */
    icom_setSockopt(socket->socket, ZMQ_SNDHWM, 1);

    if( zmq_connect(socket->socket, socket->string) ){
        _SE("Failed to bind ZMQ socket to \"%s\" string", socket->string);
        return -1;
    }

    return 0;
}

int icom_initPullSocket(icomSocket_t *socket, char *string){
    socket->string = string;
    socket->inproc = (strstr("inproc", string) != NULL)? 1 : 0;

    socket->socket = zmq_socket(icom_getContext(), ZMQ_PULL);
    if(socket->socket == NULL){
        _SE("Failed to create ZMQ socket");
        return -1;
    }

    /* this is the best we can do in terms of PUSH syncronization */
    icom_setSockopt(socket->socket, ZMQ_RCVHWM, 1);

    if( zmq_bind(socket->socket, socket->string) ){
        _SE("Failed to bind ZMQ socket to \"%s\" string", socket->string);
        return -1;
    }

    return 0;
}

int icom_initPullSockets(icomSocket_t **sockets, unsigned socketCount, char** comStrings){
    *sockets = (icomSocket_t*)malloc(socketCount*sizeof(icomSocket_t));

    for(int i=0; i<socketCount; i++)
        icom_initPullSocket(*sockets+i, comStrings[i]);

    return 0;
}

int icom_initPushSockets(icomSocket_t **sockets, unsigned socketCount, char** comStrings){
    *sockets = (icomSocket_t*)malloc(socketCount*sizeof(icomSocket_t));

    for(int i=0; i<socketCount; i++)
        icom_initPushSocket(*sockets+i, comStrings[i]);

    return 0;
}

int icom_initBuffers(icomBuffer_t **buffers, unsigned bufferCount, unsigned bufferSize){
    *buffers = (icomBuffer_t*)malloc(bufferCount*sizeof(icomBuffer_t));

    for(int i=0; i<bufferCount; i++){
        (*buffers)[i].size = bufferSize;
        (*buffers)[i].mem  = malloc(bufferSize);
    }

    /* create a linked list */
    for(int i=0; i<bufferCount-1; i++){
        (*buffers)[i].next = &((*buffers)[i+1]);
    }
    (*buffers)[bufferCount-1].next = NULL;

    return 0;
}

icom_t *icom_initPush(char *comString, unsigned bufferSize, unsigned bufferCount, uint32_t flags){

    /* allocate and initialize icom structure */
    icom_t *icom = (icom_t*)malloc(sizeof(icom_t));
    icom->bufferCount = bufferCount;
    icom->bufferIdx   = 0;
    icom->type        = ICOM_TYPE_PUSH;
    icom->cbDo        = icom_doPush;

    /* parse communication strings */
    comStringGetArray(&(icom->comStrings), &(icom->socketCount), comString);

    /* initialize sockets */
    icom_initPushSockets(&(icom->sockets), icom->socketCount, icom->comStrings);

    /* initialize buffers */
    icom_initBuffers(&(icom->buffers), icom->bufferCount, bufferSize);

    return icom;
}

icom_t *icom_initPull(char *comString, unsigned bufferSize, uint32_t flags){

    /* allocate and initialize icom structure */
    icom_t *icom = (icom_t*)malloc(sizeof(icom_t));
    icom->type        = ICOM_TYPE_PULL;
    icom->cbDo        = icom_doPull;

    /* parse communication strings */
    comStringGetArray(&(icom->comStrings), &(icom->socketCount), comString);
    icom->bufferCount = icom->socketCount;

    /* initialize sockets */
    icom_initPullSockets(&(icom->sockets), icom->socketCount, icom->comStrings);

    /* initialize buffers */
    icom_initBuffers(&(icom->buffers), icom->bufferCount, bufferSize);

    return icom;
}

void icom_pushDeinit(icom_t *icom){
    /* release memory buffers */
    for(int i=0; i<icom->bufferCount; i++)
        free(icom->buffers[i].mem);

    for(int i=0; i<icom->socketCount; i++)
        zmq_close(icom->sockets[i].socket);

    /* release memory and socket buffers */
    free(icom->buffers);
    free(icom->sockets);

    /* release allocated strings */
    comStringClean(&(icom->comStrings), icom->socketCount);

    /* release holding struct */
    free(icom);
}

static inline int icom_sendSync(void *socket){
    char dummy[1];
    return zmq_send(socket, dummy, sizeof(dummy), 0);
}

static inline int icom_recvSync(void *socket){
    char dummy[1];
    return zmq_recv(socket, dummy, sizeof(dummy), 0);
}

static inline int icom_sendBuf(icomSocket_t *socket, icomBuffer_t *buffer){
    return zmq_send(socket->socket, buffer->mem, buffer->size, 0);
}

static inline int icom_recvBuf(icomSocket_t *socket, icomBuffer_t *buffer){
    return zmq_recv(socket->socket, buffer->mem, buffer->size, 0);
}

icomBuffer_t *icom_doPush(icom_t *icom){
    //int res;

    /* send current buffer */
    for(int i=0; i<icom->socketCount; i++){
        icom_sendBuf(&(icom->sockets[i]), &(icom->buffers[icom->bufferIdx]));

        /*  */
        //if(res == -1)
        //    _W("Message dropped");
    }

    /* update buffer index */
    if(++icom->bufferIdx >= icom->bufferCount)
        icom->bufferIdx = 0;
    
    return &(icom->buffers[icom->bufferIdx]);
}

icomBuffer_t *icom_doPull(icom_t *icom){
    /* receive all buffers from all sockets */
    for(int i=0; i<icom->socketCount; i++){
        icom_recvBuf(&(icom->sockets[i]), &(icom->buffers[i]));
    }

    return icom->buffers;
}

icomBuffer_t *icom_getCurrentBuffer(icom_t *icom){
    return &(icom->buffers[icom->bufferIdx]);
}
