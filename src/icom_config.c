#include <stdint.h>
#include "icom.h"
#include "icom_config.h"


/* list of default values available for configuration */
static uint64_t timeout_rcv_usec = 1000000; // 1 second
static uint64_t timeout_snd_usec = 1000000; // 1 second


/* configuration setter procedures */
static icomStatus_t set_uint64_t(icomConfig_t configId, void *configVal, void *inputVal){
  *(uint64_t*)configVal = *(uint64_t*)inputVal;
  return ICOM_SUCCESS;
}


/* configuration getter procedures */
static icomStatus_t get_uint64_t(icomConfig_t configId, void *configVal, void *outputVal){
  *(uint64_t*)outputVal = *(uint64_t*)configVal;
  return ICOM_SUCCESS;
}


/* static object for managing configuration setters and getters */
struct config_t {
  void *address;
  icomStatus_t (*setter)(icomConfig_t configId, void *configVal, void *inputVal);
  icomStatus_t (*getter)(icomConfig_t configId, void *configVal, void *inputVal);
};

static struct config_t config[] = {
  {&timeout_rcv_usec, set_uint64_t, get_uint64_t},
  {&timeout_snd_usec, set_uint64_t, get_uint64_t},
};


/* configuration interface */
icomStatus_t icom_setDefaultConfig(icomConfig_t configId, void *configVal){
  return config[configId].setter(configId, config[configId].address, configVal);
}

icomStatus_t icom_getDefaultConfig(icomConfig_t configId, void *configVal){
  return config[configId].getter(configId, config[configId].address, configVal);
}
