#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <zmq.h>

#include "icom.h"
#include "string_parser.h"

/* INFO */
#define _I(fmt,args...)    printf(fmt "\n", ##args); fflush(stdout)
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
    #define _D(fmt,args...)  printf("DEBUG:%u: "fmt "\n", __LINE__, ##args); fflush(stdout)
#else
    //#define _D(fmt,args...)  printf("DEBUG:%u: "fmt "\n", __LINE__, ##args); fflush(stdout)
    #define _D(fmt,args...)    
#endif


/*************** Globals ***************/
static void *zmqContext = NULL;        // single context per application
static pthread_mutex_t lockGetContext; // used for thread save initialization



/*============================================================================*/
/*                            ICOM COMMON - UTILS                             */
/*============================================================================*/
/*@ TODO */
static inline int hasAllocatedBuffers(icom_t *icom){
    if((icom->type = ICOM_TYPE_PULL) && (icom->flags & ICOM_ZERO_COPY))
        return 0;

    if((icom->type = ICOM_TYPE_SUB)  && (icom->flags & ICOM_ZERO_COPY))
        return 0;

    return 1;
}


/*@ TODO */
static inline int shouldInitializeSemaphores(icom_t *icom){
    if( !(icom->flags & ICOM_ZERO_COPY) )
        return 0;

    if( !(icom->flags & ICOM_PROTECTED) )
        return 0;

    if(icom->type == ICOM_TYPE_PULL)
        return 0;

    if(icom->type == ICOM_TYPE_SUB)
        return 0;

    return 1;
}


/*@ Initializes (if not initialized) and returns global ZMQ context */
static void *getContext(){
    pthread_mutex_lock(&lockGetContext);

    if(zmqContext == NULL){
        zmqContext = zmq_ctx_new();

        if(zmqContext == NULL)
            _SE("Failed to initialize ZMQ context");
    }

    pthread_mutex_unlock(&lockGetContext);

    return zmqContext;
}


/*@ TODO */
void setSockopt(void *socket, int name, int value){
    if(zmq_setsockopt(socket, name, &value, sizeof(value)) == -1)
        _SW("Failed to set socket option");
}

/*============================================================================*/
/*                             ICOM COMMON - API                              */
/*============================================================================*/
void icom_init(){
    getContext();  // initializes zmq context
}

void icom_release(){
    zmq_ctx_destroy(zmqContext);
}

void icom_deinit(icom_t *icom){
    /* release memory buffers, but and check  */
    if(hasAllocatedBuffers(icom)){
        for(int i=0; i<icom->packetCount; i++){
            free(icom->packets[i].payload);

            // destroy semaphores if any (TODO: refactor)
            if( hasAllocatedBuffers(icom) && 
                (icom->flags & ICOM_ZERO_COPY) &&
                (icom->flags & ICOM_PROTECTED)){
                sem_destroy(&icom->packets[i].semWrite);
                sem_destroy(&icom->packets[i].semRead);
            }
        }
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
int icom_initPackets(icom_t *icom, unsigned payloadSize){
    icom->packets = (icomPacket_t*)malloc(icom->packetCount*sizeof(icomPacket_t));

    /* initialize packets */
    for(int i=0; i<icom->packetCount; i++){
        //(*packets)[i].header.type = ; TODO
        (icom->packets)[i].header.size = payloadSize;
        (icom->packets)[i].payload     = malloc(payloadSize);
    }

    /* initialize semaphores */
    if( shouldInitializeSemaphores(icom)){
        _I("Initializing semaphores");
        for(int i=0; i<icom->packetCount; i++){
            sem_init(&icom->packets[i].semWrite, 0, 1);
            sem_init(&icom->packets[i].semRead,  0, icom->socketCount);
        }
    }

    /* create a linked list */
    for(int i=0; i<icom->packetCount-1; i++){
        (icom->packets)[i].next = &((icom->packets)[i+1]);
    }
    (icom->packets)[icom->packetCount-1].next = NULL;

    return 0;
}


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

int icom_sendPacketZero(icomSocket_t *socket, icomPacket_t *packet){
    _D("Sending zero copy packet");
    return zmq_send(socket->socket, packet, sizeof(icomPacket_t), 0);
}

int icom_recvPacketZero(icomSocket_t *socket, icomPacket_t *packet){
    _D("Receiving zero-copy packet");
    return zmq_recv(socket->socket, packet, sizeof(icomPacket_t), 0);
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
    _I("Push: Acquiring semaphore");
    sem_wait(&(icom->packets[icom->packetIndex].semWrite));
    
    /* wait for the posted read semaphores (TODO: avoid polling) */
    _I("Push: Checking read semaphore value");
    int sem_value;
    do{
        sem_getvalue(&(icom->packets[icom->packetIndex].semRead), &sem_value);
    } while(sem_value != icom->socketCount);

    /* send packet sequence */
    _I("Push: Sending packet sequence");
    for(int i=0; i<icom->socketCount; i++)
        icom_sendPacketZero(&(icom->sockets[i]), &(icom->packets[icom->packetIndex]));

    /* post write semaphore */
    _I("Push: Posting semaphore");
    sem_post(&(icom->packets[icom->packetIndex].semWrite));
    
    /* update buffer index */
    _I("Push: Updating packet index");
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
        _SE("Failed to bind ZMQ socket to \"%s\" string", socket->string);
        return -1;
    }

    return 0;
}

int icom_initPushSockets(icomSocket_t **sockets, unsigned socketCount, char** comStrings){
    *sockets = (icomSocket_t*)malloc(socketCount*sizeof(icomSocket_t));

    for(int i=0; i<socketCount; i++)
        icom_initPushSocket(*sockets+i, comStrings[i]);

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
    parser_initStrArray(&(icom->comStrings), &(icom->socketCount), comString);

    /* initialize sockets */
    icom_initPushSockets(&(icom->sockets), icom->socketCount, icom->comStrings);

    /* initialize buffers */
    icom_initPackets(icom, payloadSize);
    //icom_initPackets(&(icom->packets), icom->packetCount, payloadSize, flags);

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
    static int not_first = 0;

    /* receive all buffers from all sockets */
    for(int i=0; i<icom->socketCount; i++){
        _I("Pull: Posting previous semaphore");
        if(not_first){ // otherwise can result in an error
            sem_post(&(icom->packets[i].semRead));
        }

        _I("Pull: Receiving buffers");
        icom_recvPacketZero(&(icom->sockets[i]), &(icom->packets[i]));

        _I("Pull: Acquiring semaphore");
        sem_wait(&(icom->packets[i].semRead));
    }

    /* refactor */
    not_first = 1;

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

    for(int i=0; i<socketCount; i++)
        icom_initPullSocket(*sockets+i, comStrings[i]);

    return 0;
}

icom_t *icom_initPull(char *comString, unsigned payloadSize, uint32_t flags){
    /* allocate and initialize icom structure */
    icom_t *icom = (icom_t*)malloc(sizeof(icom_t));
    icom->flags  = flags;
    icom->type   = ICOM_TYPE_PULL;

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


    /* parse communication strings */
    parser_initStrArray(&(icom->comStrings), &(icom->socketCount), comString);
    icom->packetCount = icom->socketCount;

    /* initialize sockets */
    icom_initPullSockets(&(icom->sockets), icom->socketCount, icom->comStrings);

    /* initialize buffers */
    icom_initPackets(icom, payloadSize);
    //icom_initPackets(&(icom->packets), icom->packetCount, payloadSize, flags);

    return icom;
}

//static inline int icom_sendSync(void *socket){
//    char dummy[1];
//    return zmq_send(socket, dummy, sizeof(dummy), 0);
//}
//
//static inline int icom_recvSync(void *socket){
//    char dummy[1];
//    return zmq_recv(socket, dummy, sizeof(dummy), 0);
//}
//
//static inline int icom_sendBuf(icomSocket_t *socket, icom
//    return zmq_send(socket->socket, buffer->mem, buffer->size, 0);
//}
//
//static inline int icom_recvBuf(icomSocket_t *socket, icomBuffer_t *buffer){
//    return zmq_recv(socket->socket, buffer->mem, buffer->size, 0);
//}
//
//
//

//int icom_updateConnectSockopt(void *socket, char *string, int name, int value){
//    if(zmq_disconnect(socket, string) != 0){
//        _SE("Failed to disconnect socket");
//        return errno;
//    }
//
//    setSockopt(socket, name, value);
//    
//    if(zmq_connect(socket, string) != 0){
//        _SE("Failed to connect socket");
//        return errno;
//    };
//
//    return 0;
//}

//int icom_updateBindSockopt(void *socket, char *string, int name, int value){
//    if(zmq_unbind(socket, string) != 0){
//        _SE("Failed to unbind socket");
//        return errno;
//    }
//
//    setSockopt(socket, name, value);
//    
//    if(zmq_unbind(socket, string) != 0){
//        _SE("Failed to unbind socket");
//        return errno;
//    };
//
//    return 0;
//}
