#ifndef _LINK_SOCKET_H_
#define _LINK_SOCKET_H_

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"

typedef struct {
  int                fd;
  int                fdAccepted;
  char              *ip;
  uint16_t           port;
  struct sockaddr_in sockaddr;
} icomLinkSocket_t;


icomStatus_t icom_initSocketConnect(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);
icomStatus_t icom_initSocketBind(icomLink_t *link, icomType_t type, const char *comString, icomFlags_t flags);
void icom_deinitSocket(icomLink_t* connection);

#endif
