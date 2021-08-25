#ifndef _LINK_SOCKET_H_
#define _LINK_SOCKET_H_

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"

typedef struct {
  int  fd;
} icomLinkSocket_t;


icomLink_t* icom_initSocketConnect(const char *comString, icomType_t type, icomFlags_t flags);

icomLink_t* icom_initSocketBind(const char *comString, icomType_t type, icomFlags_t flags);

void icom_deinitSocket(icomLink_t* connection);

#endif
