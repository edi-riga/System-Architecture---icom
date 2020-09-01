#ifndef _ICOM_H_
#define _ICOM_H_

#include <stdint.h>
#include <semaphore.h>


/*@ Flags, which can be used for the initialization of the communication 
 *  channels */
#define ICOM_DEFAULT     (0)
#define ICOM_ZERO_COPY   (1<<0)
#define ICOM_PROTECTED   (1<<1)


/*@ These are the different types of icom transactions, basically a combination
 *  from different communication paradigms, zero vs deep copy and protected vs 
 *  unprotected access. Protected refers to the zero data copy and semaphore
 *  based access protection */
typedef enum {
    ICOM_TYPE_PUSH =0,  //@
    ICOM_TYPE_PULL =1,  //@
    ICOM_TYPE_PUB  =2,  //@
    ICOM_TYPE_SUB  =3,  //@
} icomPacketType_t;


/*@ struct Header of the icom packet, in future may include additional 
 *  information, e.g. timestamp */
typedef struct {
    icomPacketType_t type;
    uint32_t         size;
} icomPacketHeader_t;


/*@ Place holder for packet payload */
typedef void* icomPacketPayload_t;


/*@ struct Intercommunication memory layout, the packet is supposed to hold,
 *  either the deep copy of the data or pointer to the shared data (zero copy)
 **/
typedef struct icomPacket_t {
    struct icomPacket_t *next;     //@ the next packet used with multiple connections
    icomPacketHeader_t   header;   //@ header, received seperately before payload
    icomPacketPayload_t  payload;  //@ the actual payload (or zero copy pointer) 
    sem_t                semWrite; //@ (optional) number of write references
    sem_t                semRead;  //@ (optional) number of read references
} icomPacket_t;


/*@ struct Descriptor of a single communication socket */
typedef struct {
    char     *string;  //@ string identifier of the communication
    void     *socket;  //@ socket descriptor
    int       inproc;  //@ flag that socket is used for in-process communication
} icomSocket_t;


/*@ struct Descriptor of a communication */
typedef struct icom_t{
    /* high-level configuration */
    icomPacketType_t  type;        //@ type of the communication
    char            **comStrings;  //@ input communication string (might be used for debugging)
    uint32_t          flags;       //@ flags associated with the communication

    /* communication socket */
    icomSocket_t     *sockets;     //@ an array of communication sockets
    unsigned          socketCount; //@ number of communication sockets

    /* packets / data payload */
    icomPacket_t     *packets;     //@ an array of communication packets (buffers)
    unsigned          packetCount; //@ number of communication packets (buffers)
    unsigned          packetIndex; //@ index of the current packet

    /* communication callbacks / implementation */
    icomPacket_t* (*cbDo)    (struct icom_t *icom); //@ communication implementation
    void          (*cbDeinit)(struct icom_t *icom); //@ communication release callback
} icom_t;



/*@ TODO */
int icom_init();


/*@ TODO */
void icom_release();


/*@ Initialization of the icom PUSH-PULL communication source
 *
 *  @param comString   TODO
 *  @param bufferSize  TODO
 *  @param bufferCount TODO
 *
 *  @return Returns icom communication descriptor */
icom_t *icom_initPush(char* comString, unsigned bufferSize, unsigned bufferCount, uint32_t flags);


/*@ Initialization of the icom PUSH-PULL communication sink
 *
 *  @param comString   TODO
 *  @param bufferSize  TODO
 *
 *  @return Returns icom communication descriptor */
icom_t *icom_initPull(char *comString, unsigned bufferSize, uint32_t flags);


/*@ Initialization of the icom PUBLISHER
 * TODO
 *
 *  @return Returns icom communication descriptor */
icom_t *icom_initPublish(
char      *comString,
unsigned   payloadSize,
unsigned   packetCount,
uint32_t   flags);


/*@ Initialization of the icom SUBSCRIBER
 *
 *  @param comString   TODO
 *  @param bufferSize  TODO
 *
 *  @return Returns icom communication descriptor */
icom_t *icom_initSubscribe(
char      *comString, 
unsigned   payloadSize, 
uint32_t   flags);


/*@ TODO */
void icom_deinit(icom_t *icom);


/*@ TODO */
//void icom_deinitPush(icom_t *icom);


/*@ TODO */
//void icom_deinitPull(icom_t *icom);


/*@ TODO */
static inline icomPacket_t *icom_do(icom_t *icom){
    return icom->cbDo(icom);
}


/*@ TODO */
icomPacket_t *icom_getCurrentPacket(icom_t *icom);


/*@ Front (begining) buffer iteration macros to comfortably iterate trough all
 * of the buffers inside a receive buffer linked list where presumably each 
 * buffer in the linked list has been received from a different thread. An 
 * example usage would be when your PULL thread anticipates transactions from
 * multiple (or just a single) threads. The macro provides a direct access to 
 * the received buffer, probably should be used when the buffer size is known
 * and static. 
 *
 * USAGE:
 * ICOM_FOR_EACH_BUFFER(packet, buffer)
 *    <do work with the buffer>
 * ICOM_FOR_EACH_BUFFER_END
 *
 * */
#define ICOM_FOR_EACH_BUFFER(packet, buffer)                                   \
do{                                                                            \
    buffer = (typeof(buffer))packet->payload;


/*@ Front (begining) buffer iteration macros to comfortably iterate trough all
 * of the buffers inside a receive buffer linked list where presumably each 
 * buffer in the linked list has been received from a different thread. An 
 * example usage would be when your PULL thread anticipates transactions from
 * multiple (or just a single) threads. The macro provides a direct access to 
 * the received buffer and its size, probably should be used in cases when the 
 * buffer size can vary between transactions and/or sources.
 *
 * USAGE:
 * ICOM_FOR_EACH_BUFFER_SIZE(packet, buffer, size)
 *    <do work with the buffer>
 * ICOM_FOR_EACH_BUFFER_END
 *
 * */
#define ICOM_FOR_EACH_BUFFER_SIZE(packet, buffer, size)                        \
do{                                                                            \
    buffer = (typeof(buffer))packet->payload;                                  \
    size   = (typeof(size))packet->header.size;


/*@ Front (begining) buffer iteration macros to comfortably perform transactions 
 * and iterate trough all of the buffers inside a receive buffer linked list 
 * where presumably each buffer in the linked list has been received from a 
 * different thread. An example usage would be when your PULL thread anticipates
 * transactions from multiple (or just a single) threads. The macro provides a 
 * direct access to the received buffer, probably should be used when the buffer
 * size is known and static. 
 *
 * USAGE:
 * ICOM_DO_FOR_EACH_BUFFER(icom, buffer)
 *    <do work with the buffer>
 * ICOM_FOR_EACH_BUFFER_END
 *
 * */
#define ICOM_DO_AND_FOR_EACH_BUFFER(icom, buffer)                              \
icomPacket_t *_tmpPacket;                                                      \
_tmpPacket = icom_do(icom);                                                    \
ICOM_FOR_EACH_BUFFER(_tmpPacket, buffer)


/*@ Front (begining) buffer iteration macros to comfortably perform transactions
 * and iterate trough all of the buffers inside a receive buffer linked list
 * where presumably each buffer in the linked list has been received from a
 * different thread. An example usage would be when your PULL thread anticipates
 * transactions from multiple (or just a single) threads. The macro provides a
 * direct access to the received buffer and its size, probably should be used in
 * cases when the buffer size can vary between transactions and/or sources.
 *
 * USAGE:
 * ICOM_DO_FOR_EACH_BUFFER(icom, buffer, size)
 *    <do work with the buffer>
 * ICOM_FOR_EACH_BUFFER_END
 *
 * */
#define ICOM_DO_AND_FOR_EACH_BUFFER_SIZE(icom, buffer, size)                   \
icomPacket_t *_tmpPacket;                                                      \
_tmpPacket = icom_do(icom);                                                    \
ICOM_FOR_EACH_BUFFER_SIZE(_tmpPacket, buffer, size)


/*@ Back (ending) buffer iteration macros, used to terminate any previously
 * defined iteration macro.
 *
 * */
#define ICOM_FOR_EACH_END                                                      \
    _tmpPacket = _tmpPacket->next;                                             \
}                                                                              \
while( _tmpPacket!=NULL );


/*@ Macro attempts to perform a transaction of the previously configured icom 
 * communication. By a large extent the macro is not necessary but is provided
 * the symetry of the API.
 * */
#define ICOM_DO(icom)                                                          \
icom_do(icom);


/*@ Macro retreiving buffer (payload) from the icom communication descriptor. By
 * a large extent the macro is not necessary but is provided the symetry of the 
 * API.
 * */
#define ICOM_GET_BUFFER(icom, buffer)                                          \
{                                                                              \
icomPacket_t *_tmpPacket = icom_getCurrentPacket(icom);                        \
buffer = (typeof(buffer))_tmpPacket->payload;                                  \
}


/*@ Macro retreiving buffer (payload) and its size from the icom communication 
 * descriptor. By a large extent the macro is not necessary but is provided the
 * symetry of the API.
 * */
#define ICOM_GET_BUFFER_SIZE(icom, buffer, size)                               \
icomPacket_t *_tmpPacket = icom_getCurrentPacket(icom);                        \
do{                                                                            \
    buffer = (typeof(buffer))_tmpPacket->payload;                              \
    size   = (typeof(size))_tmpPacket->header.size;


#define ICOM_DO_AND_GET_BUFFER(icom, buffer)                                   \
icom_do(icom);                                                                 \
{                                                                              \
icomPacket_t *_tmpPacket = icom_getCurrentPacket(icom);                        \
buffer = (typeof(buffer))_tmpPacket->payload;                                  \
}

//TODO: Macros for buffer initialization
//#define ICOM_INIT_FOR_EACH_BUFFER(icom, buffer)                                
//icomPacket_t *_tmpPacket;                                                      
//for(int _idx=0; _idx<icom->packetCount; _idx++){                               
//    buffer = (typeof(buffer))_tmpPacket->payload;
//
//
//#define ICOM_INIT_FOR_EACH_BUFFER_END                                          
//}

#endif
