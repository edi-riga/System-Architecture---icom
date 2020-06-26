#ifndef _ICOM_PUSH_H_
#define _ICOM_PUSH_H_

/*@ struct Linked list of specialized icom buffers which are used to 
 *         implement simultanious operations of writing/reading buffers 
 *         by different threads */
typedef struct icomBuffer_t{
    struct icomBuffer_t *next; //@ next buffer, or NULL for the last buffer
    unsigned size;             //@ size of the memory buffer
    void *mem;                 //@ data
} icomBuffer_t;


/*@ struct Descriptor of a single communication socket */
typedef struct {
    char *string;  //@ string identifier of the communication
    void *socket;  //@ socket descriptor
    int  inproc;   //@ flag that socket is used for in-process communication
    struct icomEntry_t *entry;
} icomSocket_t;

/*@ struct Communication descriptor */
typedef struct icom_t{
    unsigned type;          //@ communication type 
    unsigned socketCount;   //@ number of associated sockets
    unsigned bufferCount;   //@ number of associated buffers
    unsigned bufferIdx;     //@ index of the current buffer
    char   **comStrings;    //@ communication string
    icomSocket_t *sockets;  //@ array of sokets
    icomBuffer_t *buffers;  //@ array of buffers
    icomBuffer_t* (*cbDo)(struct icom_t *icom);     //@ TODO
    void          (*cbDeinit)(struct icom_t *icom); //@ TODO
    void          (*cbTimeout)(char *str);          //@ TODO
} icom_t;

/*@ Initialization of the icom PUSH-PULL communication source
 *
 *  @param comString   TODO
 *  @param bufferSize  TODO
 *  @param bufferCount TODO
 *
 *  @return Returns icom communication descriptor */
icom_t *icom_initPush(ch* comString, unsigned bufferSize, unsigned bufferCount);


/*@ Initialization of the icom PUSH-PULL communication sink, note that 
 *      there is no need for bufferSize
 *
 *  @param comString   TODO
 *  @param bufferSize  TODO
 *
 *  @return Returns icom communication descriptor */
icom_t *icom_initPull(char *comString, unsigned bufferSize);


/*@ TODO */
void icom_release();


/*@ TODO */
void icom_pushDeinit(icom_t *icom);


/*@ TODO */
static inline icomBuffer_t *icom_do(icom_t *icom){
    return icom->cbDo(icom);
}


/*@ TODO */
static inline void icom_deinit(icom_t *icom){
    icom->cbDeinit(icom);
}

/*@ TODO */
icomBuffer_t *icom_getCurrentBuffer(icom_t *icom);


/*@ TODO */
icomBuffer_t *icom_doPush(icom_t *icom);


/*@ TODO */
icomBuffer_t *icom_doPull(icom_t *icom);
#endif
