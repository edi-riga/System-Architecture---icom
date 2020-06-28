#ifndef _ICOM_PUSH_H_
#define _ICOM_PUSH_H_

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
    ICOM_TYPE_PUSH_DEEP            =0,  //@
    ICOM_TYPE_PUSH_ZERO_PROTECTED  =1,  //@
    ICOM_TYPE_PUSH_ZERO_UNPROTECTED=2,  //@
    ICOM_TYPE_PULL_DEEP            =3,  //@
    ICOM_TYPE_PULL_ZERO_PROTECTED  =4,  //@
    ICOM_TYPE_PULL_ZERO_UNPROTECTED=5,  //@
    ICOM_TYPE_PUB_DEEP             =6,  //@
    ICOM_TYPE_PUB_ZERO_PROTECTED   =7,  //@
    ICOM_TYPE_PUB_ZERO_UNPROTECTED =8,  //@
    ICOM_TYPE_SUB_DEEP             =9,  //@
    ICOM_TYPE_SUB_ZERO_PROTECTED   =10, //@
    ICOM_TYPE_SUB_ZERO_UNPROTECTED =11, //@
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
void icom_init();


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


/*@ Initialization of the icom PUSH-PULL communication sink, note that 
 *      there is no need for bufferSize
 *
 *  @param comString   TODO
 *  @param bufferSize  TODO
 *
 *  @return Returns icom communication descriptor */
icom_t *icom_initPull(char *comString, unsigned bufferSize, uint32_t flags);


/*@ TODO */
void icom_deinit(icom_t *icom);


/*@ TODO */
void icom_deinitPush(icom_t *icom);


/*@ TODO */
void icom_deinitPull(icom_t *icom);


/*@ TODO */
static inline icomPacket_t *icom_do(icom_t *icom){
    return icom->cbDo(icom);
}


/*@ TODO */
icomPacket_t *icom_getCurrentPacket(icom_t *icom);


///*@ TODO */
//static inline void icom_deinit(icom_t *icom){
//    icom->cbDeinit(icom);
//}
//
//
//
///*@ TODO */
//icomPacket_t *icom_doPush(icom_t *icom);
//
//
///*@ TODO */
//icomPacket_t *icom_doPull(icom_t *icom);


/******************** Legacy ********************/

// legacy
//typedef struct icomLBuffer_t {
//    struct icomLBuffer_t *next;
//    uint32_t              size;
//    void                 *data;
//} icomLBuffer_t;

/*@ struct Linked list of specialized icom buffers which are used to 
 *         implement simultanious operations of writing/reading buffers 
 *         by different threads */
//typedef struct icomBuffer_t{
//    struct icomBuffer_t *next; //@ next buffer, or NULL for the last buffer
//    unsigned size;             //@ size of the memory buffer
//    void *mem;                 //@ data
//} icomBuffer_t;


/*@ struct Descriptor of a single communication socket */
//typedef struct {
//    char *string;  //@ string identifier of the communication
//    void *socket;  //@ socket descriptor
//    int  inproc;   //@ flag that socket is used for in-process communication
//    struct icomEntry_t *entry;
//} icomSocket_t;

/*@ struct Communication descriptor */
//typedef struct icom_t{
//    unsigned type;          //@ communication type 
//    unsigned socketCount;   //@ number of associated sockets
//    unsigned bufferCount;   //@ number of associated buffers
//    unsigned bufferIdx;     //@ index of the current buffer
//    char   **comStrings;    //@ communication string
//    icomSocket_t *sockets;  //@ array of sokets
//    icomBuffer_t *buffers;  //@ array of buffers
//    icomBuffer_t* (*cbDo)(struct icom_t *icom);     //@ TODO
//    void          (*cbDeinit)(struct icom_t *icom); //@ TODO
//    void          (*cbTimeout)(char *str);          //@ TODO
//} icom_t;


#endif
