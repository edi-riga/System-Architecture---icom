#include <stdio.h>
#include <stdlib.h>
#include "icom.h"
#include "notification.h"
#include "string_parser.h"


icom_t* icom_init(const char *comString, icomType_t type, icomFlags_t flags){
  int ret;
  icom_t *icom;

  /* allocate and initialize icom structure */
  icom = (icom_t*)malloc(sizeof(icom_t));
  if(!icom){
    _E("Failed to allocate memory");
    return NULL;
  }

  /* parse communication strings */
  ret = parser_initStrArray(&icom->comStrings, &icom->comCount, comString);
  if(ret != 0){
    _E("Failed to parse communication string");
    goto failure_initStrArray;
  }

  /* TODO: initialize selected icom communication */


  return icom;

failure_initStrArray:
  free(icom);
  return NULL;
}


void icom_deinit(icom_t* icom){

  /* TODO: deallocate icom structure  */

  /* deallocate icom structure  */
  free(icom);
}
