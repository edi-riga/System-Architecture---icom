#ifndef _LINK_FIFO_H_
#define _LINK_FIFO_H_

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"

typedef struct {
  int  fd;
} icomLinkFifo_t;

icomLink_t* icom_initFifo(const char *comString, icomType_t type, icomFlags_t flags);
void icom_deinitFifo(icomLink_t* connection);

#endif

