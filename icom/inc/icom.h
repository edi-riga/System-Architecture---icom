#ifndef _ICOM_H_
#define _ICOM_H_

/* force C linkage if included from C++ */
#ifdef __cplusplus
  extern "C" {
#endif


#include <stdint.h>
#include "icom_type.h"
#include "icom_flags.h"
#include "icom_status.h"
#include "icom_macro.h"

/* forward declarations */
typedef struct icomLink icomLink_t;
typedef struct icom icom_t;


/** @brief The main icom (internal communication) encapsulation object */
typedef struct icom {
  icomType_t    type;            /** communication type */
  icomFlags_t   flags;           /** communication flags */
  unsigned      comCount;        /** number of communication links */
  icomLink_t   *comConnections;  /** communication links */
  char        **comStrings;      /** strings for the communication links */
} icom_t;

/** @brief The header of any communication link which is sent before any
 *  actual data transfer */
typedef struct {
  icomType_t   type;    /** communication type (4 bytes) */
  icomFlags_t  flags;   /** communication flags (4 bytes) */
  uint32_t     bufSize; /** upcomming buffer size (4 bytes) */
} icomMsgHeader_t;

/** @brief Generic encapsulation object for any communication link */
typedef struct icomLink {
  const char  *comString;   /** communication string */
  void        *pdata;       /** private communication link's data */
  icomType_t   type;        /** communication type */
  icomFlags_t  flags;       /** communication flags */
  void        *recvBuf;     /** points to received data buffer (recvBuf-sizeof(pointer)
                                holds pointer to the respective link structure) */
  uint32_t     recvSize;    /** number of bytes required to receive data from the
                                sender, may correspond to pointer size when zero-copying */
  uint32_t     recvBufSize; /** number of bytes in the received buffer,
                                corresponds to the actual sender buffer size */
  icomStatus_t (*sendHandler)(icomLink_t *link, void *buf, unsigned bufSize);
  icomStatus_t (*sendHandlerSecondary)(icomLink_t *link, void *buf, unsigned bufSize);
  icomStatus_t (*recvHandler)(icomLink_t *link, void **buf, unsigned *bufSize);
  icomStatus_t (*notifySendHandler)(icomLink_t *link, void **buf, unsigned *bufSize);
  icomStatus_t (*notifyRecvHandler)(icomLink_t *link, void **buf, unsigned *bufSize);
  icomStatus_t (*autoSendAck)(icomLink_t *link, void **buf, unsigned *bufSize);
  icomStatus_t (*autoRecvAck)(icomLink_t *link, void **buf, unsigned *bufSize);
} icomLink_t;


/** @brief Initializes icom communication object. The communication object
 *         supports multiple and different links, e.g. socket, fifo, etc.
 *         The message can be sent to all links by using the icom_send routine.
 *
 *  @param comString String used for the actual creation of the communication
 *         links. Description of a single link is expressed as
 *         "type|flags|comId", for example, a standard Linux socket can be
 *         created with "socket_tx|default|:127.0.0.1:99999". Multiple
 *         connections can be generated by either comma-separating individual
 *         communications or by using special range syntax:
 *         "socket_tx:127.0.0.1:[9988-9999]". _flags_ parameter determines the
 *         underlying configuration of the links (synchroniztion, zero copy).
 *
 *  @return On success returns an icom object. Otherwise on error, the
 *        ICOM_IS_ERR(ptr) returns true, and the ICOM_PTR_ERR(ptr)
 *        returns the actual icomStatus_t error code.
 */
icom_t* icom_init(const char *comString);

/** @brief Deinitializes icom communication object.
 *
 *  @param icom Pointer to the icom communication object.
 */
void icom_deinit(icom_t* icom);

icomStatus_t icom_do(icom_t *icom);
icomStatus_t icom_send(icom_t *icom, void  *buf, unsigned bufSize);
icomStatus_t icom_recv1(icom_t *icom);
icomStatus_t icom_recv2(icom_t *icom, void **buf);
icomStatus_t icom_recv3(icom_t *icom, void **buf, unsigned *bufSize);
icomStatus_t icom_notify_send(icom_t *icom);
icomStatus_t icom_notify_recv(icom_t *icom);

#define icom_recv(...) \
  CONCATENATE(icom_recv,ARGUMENT_COUNT(__VA_ARGS__)(__VA_ARGS__))

void* icom_nextBuffer(icom_t *icom, void **buf, unsigned *bufSize);

icomStatus_t icom_setBuffer2(icom_t *icom, void *buf);
icomStatus_t icom_setBuffer3(icom_t *icom, void *buf, unsigned bufSize);
icomStatus_t icom_getBuffer2(icom_t *icom, void **buf);
icomStatus_t icom_getBuffer3(icom_t *icom, void **buf, unsigned *bufSize);

// TODO: API to receive buffer count

#define icom_setBuffer(...) \
  CONCATENATE(icom_setBuffer,ARGUMENT_COUNT(__VA_ARGS__)(__VA_ARGS__))

#define icom_getBuffer(...) \
  CONCATENATE(icom_getBuffer,ARGUMENT_COUNT(__VA_ARGS__)(__VA_ARGS__))


/* force C linkage if included from C++ - STOP */
#ifdef __cplusplus
  }
#endif

#endif
