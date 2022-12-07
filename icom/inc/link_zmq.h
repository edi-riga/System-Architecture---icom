#ifndef _LINK_ZMQ_H_
#define _LINK_ZMQ_H_

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"

typedef struct {
  int  fd;
} icomLinkZmq_t;


icomStatus_t icom_initZmqPush(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);
icomStatus_t icom_initZmqPull(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);
icomStatus_t icom_initZmqPub (icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);
icomStatus_t icom_initZmqSub (icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);
icomStatus_t icom_initZmqReq (icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);
icomStatus_t icom_initZmqRep (icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);

void icom_deinitZmqPush(icomLink_t* link);
void icom_deinitZmqPull(icomLink_t* link);
void icom_deinitZmqPub (icomLink_t* link);
void icom_deinitZmqSub (icomLink_t* link);
void icom_deinitZmqReq (icomLink_t* link);
void icom_deinitZmqRep (icomLink_t* link);

#endif
