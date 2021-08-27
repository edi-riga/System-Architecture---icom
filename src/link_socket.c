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


icomLink_t* icom_initSocketConnect(const char *comString, icomType_t type, icomFlags_t flags){
  icomLink_t *link;
  icomLink_t *ret;
  icomLinkSocket_t *pdata;
  uint16_t port;
  const char *p;
  char ip[sizeof("xxx.xxx.xxx.xxx")];

  /* check if IP portion of the comunication string is correct */
  p = comString;
  while( ((p-comString) < sizeof(ip)) && (*p++ != ':') );
  if((p-comString) >= sizeof(ip)){
    _E("Failed to parse communication string");
    return (icomLink_t*)ICOM_EINVAL;
  }

  /* retreive communication information */
  int r = sscanf(comString, "%[^:]:%hu", ip, &port);
  if(r != 2){
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

  if(connect(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1){
    _SE("Failed to connect socket");
    ret = (icomLink_t*)(long long)ICOM_ELINK;
    goto failure_connect;
  }

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

  /* check if IP portion of the comunication string is correct */
  p = comString;
  while( ((p-comString) < sizeof(ip)) && (*p++ != ':') );
  if((p-comString) >= sizeof(ip)){
    _E("Failed to parse communication string");
    return (icomLink_t*)ICOM_EINVAL;
  }

  /* retreive communication information */
  int r = sscanf(comString, "%[^:]:%hu", ip, &port);
  if(r != 2){
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

  if(bind(pdata->fd, (struct sockaddr*)&pdata->sockaddr, sizeof(struct sockaddr_in)) == -1){
    _SE("Failed to bind socket");
    ret = (icomLink_t*)(long long)errno;
    goto failure_bind;
  }

  pdata->ip   = strdup(ip);
  pdata->port = port;
  link->pdata = pdata;
  link->flags = flags;
  link->type  = type;

  return link;


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
  free (((icomLinkSocket_t*)(link->pdata))->ip);
  close(((icomLinkSocket_t*)(link->pdata))->fd);
  free(link->pdata);
  free(link);
}
