#ifndef _ICOM_H_
#define _ICOM_H_

#include <stdint.h>

typedef enum {
  ICOM_TYPE_PUSH=0,
  ICOM_TYPE_PULL,
  ICOM_TYPE_REQ,
  ICOM_TYPE_REP,
} icomType_t;

typedef enum {
  ICOM_SUCCESS=0,
  ICOM_ERROR,
} icomStatus_t;

typedef uint32_t icomFlags_t;

typedef struct {
  icomType_t    type;
  icomFlags_t   flags;
  char         *comStrings;
  unsigned      comCount;
} icom_t;



icom_t* icom_init(const char *comString, icomType_t type, icomFlags_t flags);
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
