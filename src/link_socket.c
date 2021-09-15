#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "icom.h"
#include "icom_type.h"
#include "icom_status.h"
#include "link_socket.h"
#include "notification.h"
#include "config.h"
#include "macro.h"


static icomStatus_t link_send(icomLink_t *link, void *buf, unsigned bufSize){
  icomMsgHeader_t header;
  int ret;

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Construct and send header */
  header.type    = ICOM_TYPE_SOCKET_TX;
  header.flags   = 0;
  header.bufSize = bufSize;
  ret = send(pdata->fd, &header, sizeof(header), 0);
  if(ret == -1){
    _SE("Send failed (header)");
    return ICOM_ERROR;
  }

  /* Send data */
  ret = send(pdata->fd, buf, bufSize, 0);
  if(ret == -1){
    _SE("Send failed (data)");
    return ICOM_ERROR;
  }

  /* Check if size of requested and sent data is equal */
  if(ret != bufSize){
    return ICOM_PARTIAL;
  }

  return ICOM_SUCCESS;
}


static icomStatus_t link_recv(icomLink_t *link, void **buf,  unsigned *bufSize){
  icomMsgHeader_t header;
  int ret;

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Receive header */
  ret = recv(pdata->fdAccepted, &header, sizeof(header), 0);
  if(ret == -1){
    _SE("Send failed (header)");
    return ICOM_ERROR;
  }

  /* Reallocate input buffer */
  if(link->recvBufSize != *bufSize){
    link->recvBufSize = *bufSize;
    link->recvBuf     = (void*)realloc(link->recvBuf, link->recvBufSize);
  }

  /* Receive the actual data */
  ret = recv(pdata->fdAccepted, link->recvBuf, link->recvBufSize, 0);
  if(ret == -1){
    _SE("Send failed (header)");
    return ICOM_ERROR;
  }

  /* Setup output arguments */
  *buf     = link->recvBuf;
  *bufSize = ret;

  /* Check if size of requested and sent data is equal */
  if(ret != link->recvBufSize){
    return ICOM_PARTIAL;
  }

  return ICOM_SUCCESS;
}


static icomStatus_t link_connectAndSend(icomLink_t *link, void *buf, unsigned bufSize){

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Connect to the */
  if(connect(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1){
    _SE("Failed to connect socket");
    if(errno == ECONNREFUSED){
      return ICOM_ECONNREFUSED;
    } else {
      return ICOM_ERROR;
    }
  }

  /* Assuming we are connected, switch future handler just send */
  link->sendHandler = link_send;

  /* Call send handler */
  return link->sendHandler(link, buf, bufSize);
}

static icomStatus_t link_acceptAndRecv(icomLink_t *link, void **buf, unsigned *bufSize){

  /* Retreive private data structure */
  icomLinkSocket_t *pdata = link->pdata;

  /* Connect to the */
  pdata->fdAccepted = accept(pdata->fd, NULL, NULL);
  if(pdata->fdAccepted == -1){
    _SE("Failed to accept socket");
    return ICOM_ERROR;
  }

  /* Assuming we have an accepted socket, switch future handler to just recv */
  link->recvHandler = link_recv;

  /* Call send handler */
  return link->recvHandler(link, buf, bufSize);
}


static icomStatus_t link_sendNotImplemented(icomLink_t *link, void *buf, unsigned bufSize){
  return ICOM_NIMPL;
}

static icomStatus_t link_recvNotImplemented(icomLink_t *link, void **buf, unsigned *bufSize){
  return ICOM_NIMPL;
}


icomLink_t* icom_initSocketConnect(const char *comString, icomType_t type, icomFlags_t flags){
  icomLink_t *link;
  icomLink_t *ret;
  icomLinkSocket_t *pdata;
  uint16_t port;
  const char *p;
  char ip[sizeof("xxx.xxx.xxx.xxx")];
  char typeString[MAX_TYPE_STRING_LENGTH+1];
  int not_connected = 1;

  /* check if IP portion of the comunication string is correct */
  p = comString;
  while( ((p-comString) < sizeof(ip)) && (*p++ != ':') );
  if((p-comString) >= sizeof(ip)){
    _E("Failed to parse communication string");
    return (icomLink_t*)ICOM_EINVAL;
  }

  /* retreive communication information */
  int r = sscanf(comString, "%" STR(MAX_TYPE_STRING_LENGTH) "[^:]:%15[^:]:%hu", typeString, ip, &port);
  if(r != 3){
    _E("Failed to parse communication string");
    return (icomLink_t*)ICOM_EINVAL;
  }

  /* allocating memory for the link data structure */
  link = (icomLink_t*)malloc(sizeof(icomLink_t));
  if(!link){
    _E("Failed to allocate memory");
    return (icomLink_t*)ICOM_ENOMEM;
  }

  /* allocating memory for the private link data structure */
  pdata = (icomLinkSocket_t*)malloc(sizeof(icomLinkSocket_t));
  if(!pdata){
    _E("Failed to allocate memory");
    ret = (icomLink_t*)ICOM_ENOMEM;
    goto failure_malloc_linkSocket;
  }

  /* creating TCP socket */
  pdata->fd = socket(AF_INET, SOCK_STREAM, 0);
  if(pdata->fd == -1){
    _SE("Failed to create socket");
    ret = (icomLink_t*)(long long)errno;
    goto failure_socket;
  }

  /* set sender socket */
  memset(&pdata->sockaddr, 0, sizeof(struct sockaddr_in));
  pdata->sockaddr.sin_family = AF_INET;
  pdata->sockaddr.sin_port   = htons(port);
  if(inet_aton(ip, &pdata->sockaddr.sin_addr) == 0){
    _E("Failed to convert IP address");
    ret = (icomLink_t*)ICOM_EINVAL;
    goto failure_inet_aton;
  }

  /* attempt connectn */
  if(connect(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1){
    if(errno == ECONNREFUSED){
      //_SW("Failed to connect socket");
      not_connected = 0;
    } else {
      _SE("Failed to connect socket");
      ret = (icomLink_t*)(long long)ICOM_ELINK;
      goto failure_connect;
    }
  }

  /* set up handlers */
  link->sendHandler = (not_connected) ? (link_connectAndSend) : (link_send);
  link->recvHandler = link_recvNotImplemented;

  pdata->ip   = strdup(ip);
  pdata->port = port;
  link->pdata = pdata;
  link->flags = flags;
  link->type  = type;

  return link;


failure_connect:
failure_inet_aton:
  close(pdata->fd);
failure_socket:
  free(pdata);
failure_malloc_linkSocket:
  free(link);
  return ret;
}

icomLink_t* icom_initSocketBind(const char *comString, icomType_t type, icomFlags_t flags){
  icomLink_t *link;
  icomLink_t *ret;
  icomLinkSocket_t *pdata;
  uint16_t port;
  const char *p;
  char ip[sizeof("xxx.xxx.xxx.xxx")];
  char typeString[MAX_TYPE_STRING_LENGTH+1];

  /* check if IP portion of the comunication string is correct */
  p = comString;
  while( ((p-comString) < sizeof(ip)) && (*p++ != ':') );
  if((p-comString) >= sizeof(ip)){
    _E("Failed to parse communication string");
    return (icomLink_t*)ICOM_EINVAL;
  }

  /* retreive communication information */
  int r = sscanf(comString, "%" STR(MAX_TYPE_STRING_LENGTH) "[^:]:%15[^:]:%hu", typeString, ip, &port);
  if(r != 3){
    _E("Failed to parse communication string");
    return (icomLink_t*)ICOM_EINVAL;
  }

  /* allocating memory for the link data structure */
  link = (icomLink_t*)malloc(sizeof(icomLink_t));
  if(!link){
    _E("Failed to allocate memory");
    return (icomLink_t*)ICOM_ENOMEM;
  }

  /* allocating memory for the private link data structure */
  pdata = (icomLinkSocket_t*)malloc(sizeof(icomLinkSocket_t));
  if(!pdata){
    _E("Failed to allocate memory");
    ret = (icomLink_t*)ICOM_ENOMEM;
    goto failure_malloc_linkSocket;
  }

  /* creating TCP socket */
  pdata->fd = socket(AF_INET, SOCK_STREAM, 0);
  if(pdata->fd == -1){
    _SE("Failed to create socket");
    ret = (icomLink_t*)(long long)errno;
    goto failure_socket;
  }

  /* set sender socket */
  memset(&pdata->sockaddr, 0, sizeof(struct sockaddr_in));
  pdata->sockaddr.sin_family = AF_INET;
  pdata->sockaddr.sin_port   = htons(port);
  if(strcmp(ip, "*") == 0){
    pdata->sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else if(inet_aton(ip, &pdata->sockaddr.sin_addr) == 0){
    _E("Failed to convert IP address");
    ret = (icomLink_t*)ICOM_EINVAL;
    goto failure_inet_aton;
  }

  /* bind to the IP address */
  if(bind(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1){
    _SE("Failed to bind socket");
    ret = (icomLink_t*)(long long)errno;
    goto failure_bind;
  }

  if(listen(pdata->fd, 1) == -1){
    _SE("Failed to mark socket passive");
    ret = (icomLink_t*)(long long)errno;
    goto failure_listen;
  };

  /* set up handlers */
  link->sendHandler = link_sendNotImplemented;
  link->recvHandler = link_acceptAndRecv;

  pdata->ip   = strdup(ip);
  pdata->port = port;
  link->pdata = pdata;
  link->flags = flags;
  link->type  = type;
  link->recvBuf     = NULL;
  link->recvBufSize = 0;

  return link;


failure_listen:
failure_bind:
failure_inet_aton:
  close(pdata->fd);
failure_socket:
  free(pdata);
failure_malloc_linkSocket:
  free(link);
  return ret;
}

void icom_deinitSocket(icomLink_t* link){
  if(link->recvBuf)
    free(link->recvBuf);
  free (((icomLinkSocket_t*)(link->pdata))->ip);
  close(((icomLinkSocket_t*)(link->pdata))->fd);
  free(link->pdata);
  free(link);
}
