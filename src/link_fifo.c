#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"
#include "link_fifo.h"
#include "notification.h"


icomLink_t* icom_initFifo(const char *comString, icomType_t type, icomFlags_t flags){
  return (icomLink_t*)ICOM_NIMPL;
}


void icom_deinitFifo(icomLink_t* connection){
}
