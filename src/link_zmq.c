#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"
#include "link_zmq.h"
#include "notification.h"


icomLink_t* icom_initZmqPush(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}

icomLink_t* icom_initZmqPull(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}

icomLink_t* icom_initZmqPub(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}

icomLink_t* icom_initZmqSub(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}

icomLink_t* icom_initZmqReq(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}

icomLink_t* icom_initZmqRep(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}


void icom_deinitZmqPush(icomLink_t* connection){
}

void icom_deinitZmqPull(icomLink_t* connection){
}

void icom_deinitZmqPub(icomLink_t* connection){
}

void icom_deinitZmqSub(icomLink_t* connection){
}

void icom_deinitZmqReq(icomLink_t* connection){
}

void icom_deinitZmqRep(icomLink_t* connection){
}

