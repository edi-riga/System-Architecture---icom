#ifndef _ICOM_CONFIG_H_
#define _ICOM_CONFIG_H_

#include "icom_status.h"

typedef enum {
  TIMEOUT_RCV_USEC=0,
  TIMEOUT_SND_USEC,
} icomConfig_t;


icomStatus_t icom_setDefaultConfig(icomConfig_t configId, void *configVal);
icomStatus_t icom_getDefaultConfig(icomConfig_t configId, void *configVal);

#endif
