#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"
#include "link_zmq.h"
#include "notification.h"


icomStatus_t icom_initZmqPush(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags){
  return ICOM_NIMPL;
}

icomStatus_t icom_initZmqPull(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags){
  return ICOM_NIMPL;
}

icomStatus_t icom_initZmqPub(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags){
  return ICOM_NIMPL;
}

icomStatus_t icom_initZmqSub(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags){
  return ICOM_NIMPL;
}

icomStatus_t icom_initZmqReq(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags){
  return ICOM_NIMPL;
}

icomStatus_t icom_initZmqRep(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags){
  return ICOM_NIMPL;
}


void icom_deinitZmqPush(icomLink_t* link){
}

void icom_deinitZmqPull(icomLink_t* link){
}

void icom_deinitZmqPub(icomLink_t* link){
}

void icom_deinitZmqSub(icomLink_t* link){
}

void icom_deinitZmqReq(icomLink_t* link){
}

void icom_deinitZmqRep(icomLink_t* link){
}

