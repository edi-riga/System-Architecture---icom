#ifndef _LINK_FIFO_H_
#define _LINK_FIFO_H_

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"

typedef struct {
  int  fd;
} icomLinkFifo_t;

icomStatus_t icom_initFifo(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);
void icom_deinitFifo(icomLink_t* link);

#endif

