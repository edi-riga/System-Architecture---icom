#ifndef _ICOM_COMMON_H_
#define _ICOM_COMMON_H_

/*@ Initializes (if not initialized) and returns global ZMQ context */
void *getContext();

void setSockopt(void *socket, int name, int value);
int icom_initPackets(icom_t *icom, unsigned payloadSize);



/*============================================================================*/
/*                            INLINE FUNCTIONS                                */
/*============================================================================*/
static inline int hasAllocatedBuffers(icom_t *icom){
    if((icom->type == ICOM_TYPE_PULL) && (icom->flags & ICOM_ZERO_COPY))
        return 0;

    if((icom->type ==ICOM_TYPE_SUB)  && (icom->flags & ICOM_ZERO_COPY))
        return 0;

    return 1;
}


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


static inline int shouldAllocateTopicBuffer(icom_t *icom){
    if(icom->type == ICOM_TYPE_SUB)
        return 1;

    return 0;
}


static inline int shouldUnbindSocket(icom_t *icom){
    if(icom->type == ICOM_TYPE_PULL)
        return 1;

    return 0;
}

#endif
