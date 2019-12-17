#ifndef _ICOM_PUSH_H_
#define _ICOM_PUSH_H_

typedef struct icomBuffer_t{
    struct icomBuffer_t *next;
    unsigned size;
    void *mem;
} icomBuffer_t;

typedef struct {
    char *string;
    void *socket;
    int  inproc;
    struct icomEntry_t *entry;
} icomSocket_t;

typedef struct icom_t{
    unsigned type;
    unsigned socketCount;
    unsigned bufferCount;
    unsigned bufferIdx;
    char   **comStrings;
    icomSocket_t *sockets;
    icomBuffer_t *buffers;
    icomBuffer_t* (*cbDo)(struct icom_t *icom);
    void          (*cbDeinit)(struct icom_t *icom);
    void          (*cbTimeout)(char *str);
} icom_t;

/* initialization */
void icom_release();
icom_t *icom_initPush(char *comString, unsigned bufferSize, unsigned bufferCount);
icom_t *icom_initPull(char *comString, unsigned bufferSize);
void icom_pushDeinit(icom_t *icom);

/* execution */
static inline icomBuffer_t *icom_do(icom_t *icom){
    return icom->cbDo(icom);
}
static inline void icom_deinit(icom_t *icom){
    icom->cbDeinit(icom);
}

/* utility */
icomBuffer_t *icom_getCurrentBuffer(icom_t *icom);

icomBuffer_t *icom_doPush(icom_t *icom);
icomBuffer_t *icom_doPull(icom_t *icom);
#endif
