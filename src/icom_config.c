#include <stdint.h>
#include "icom.h"
#include "icom_config.h"


/* list of default values available for configuration */
static uint64_t timeout_snd_usec = 1000000; // 1 second
static uint64_t timeout_rcv_usec = 1000000; // 1 second


/* configuration setter procedures */
static icomStatus_t timeout_rcv_usec_set(icomConfig_t configId, void *configVal){
  timeout_rcv_usec = *(uint64_t*)configVal;
  return ICOM_SUCCESS;
}

static icomStatus_t timeout_snd_usec_set(icomConfig_t configId, void *configVal){
  timeout_snd_usec = *(uint64_t*)configVal;
  return ICOM_SUCCESS;
}


/* configuration getter procedures */
static icomStatus_t timeout_rcv_usec_get(icomConfig_t configId, void *configVal){
  *(uint64_t*)configVal = timeout_rcv_usec;
  return ICOM_SUCCESS;
}

static icomStatus_t timeout_snd_usec_get(icomConfig_t configId, void *configVal){
  *(uint64_t*)configVal = timeout_snd_usec;
  return ICOM_SUCCESS;
}


/* configuration setter handlers */
icomStatus_t (*config_setters[])(icomConfig_t configId, void *configVal) = {
  timeout_rcv_usec_set,
  timeout_snd_usec_set,
};

/* configuration getter handlers */
icomStatus_t (*config_getters[])(icomConfig_t configId, void *configVal) = {
  timeout_rcv_usec_get,
  timeout_snd_usec_get,
};


icomStatus_t icom_setDefaultConfig(icomConfig_t configId, void *configVal){
  return config_setters[configId](configId, configVal);
}

icomStatus_t icom_getDefaultConfig(icomConfig_t configId, void *configVal){
  return config_getters[configId](configId, configVal);
}
