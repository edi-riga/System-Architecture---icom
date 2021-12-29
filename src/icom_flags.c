#include <string.h>
#include "icom_flags.h"
#include "notification.h"

static const char* icomFlagStrings[] = {
  "zero",      // ICOM_FLAG_ZERO
  "prot",      // ICOM_FLAG_PROT
//  "prot,zero", // ICOM_FLAG_ZERO | ICOM_FLAG_PROT
};

/* Should be elaborated when additional flags are added */
const char* icom_flagsToString(icomFlags_t flags){
  if(flags == 0){
    return "default";
  }

  if(flags > ICOM_FLAG_MAX_VALID){
    _E("Flag string representation lookup error");
    return NULL;
  }

  return icomFlagStrings[flags];
}


icomFlags_t icom_stringToFlags(const char *flagString){
  icomFlags_t flags = ICOM_FLAG_INVALID;

  if(strstr(flagString,"default")){
    flags = 0;
  }

  for(int i=0; i<sizeof(icomFlagStrings)/sizeof(*icomFlagStrings); i++){
    if(strstr(flagString, icomFlagStrings[i]) != NULL){
      flags |= (1<<i);
      flags &= (~ICOM_FLAG_INVALID); /* inefficient? */
    }
  }

  return flags;
}
