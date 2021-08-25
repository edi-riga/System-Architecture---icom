#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"
#include "link_socket.h"
#include "notification.h"


icomLink_t* icom_initSocketConnect(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}

icomLink_t* icom_initSocketBind(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}

void icom_deinitSocket(icomLink_t* connection){
}
