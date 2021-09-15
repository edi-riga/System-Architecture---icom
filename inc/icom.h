#ifndef _ICOM_H_
#define _ICOM_H_

#include <stdint.h>

#include "icom_type.h"
#include "icom_status.h"
#include "icom_config.h"

/* forward declarations */
typedef struct icomLink icomLink_t;

/* icom flags data type */
typedef uint32_t icomFlags_t;

/* icom communication header */
typedef struct {
  icomType_t   type;    // 4 bytes
  icomFlags_t  flags;   // 4 bytes
  uint32_t     bufSize; // 4 bytes
} icomMsgHeader_t;


typedef struct icomLink {
  const char  *comString;
  void        *pdata;
  icomType_t   type;
  icomFlags_t  flags;
  void        *recvBuf;
  uint32_t     recvBufSize;
  icomStatus_t (*sendHandler)(icomLink_t *link, void *buf, unsigned bufSize);
  icomStatus_t (*recvHandler)(icomLink_t *link, void **buf, unsigned *bufSize);
} icomLink_t;

typedef struct {
  icomType_t    type;
  icomFlags_t   flags;
  unsigned      comCount;
  icomLink_t  **comConnections;
  char        **comStrings;
} icom_t;



icom_t* icom_init(const char *comString, icomFlags_t flags);
void icom_deinit(icom_t* icom);

icomStatus_t icom_do(icom_t *icom);
icomStatus_t icom_send(icom_t *icom, void  *buf, unsigned bufSize);
icomStatus_t icom_recv(icom_t *icom, void **buf, unsigned *bufSize);

icomStatus_t icom_setBuffer2(icom_t *icom, void *buf);
icomStatus_t icom_setBuffer3(icom_t *icom, void *buf, unsigned bufSize);
icomStatus_t icom_getBuffer2(icom_t *icom, void **buf);
icomStatus_t icom_getBuffer3(icom_t *icom, void **buf, unsigned *bufSize);


#define icom_setBuffer(...) \
  CONCATENATE(icom_setBuffer,ARGUMENT_COUNT(__VA_ARGS__)(__VA_ARGS__)

#define icom_getBuffer(...) \
  CONCATENATE(icom_getBuffer,ARGUMENT_COUNT(__VA_ARGS__)(__VA_ARGS__)

#endif
