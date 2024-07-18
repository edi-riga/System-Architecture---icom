#include <string.h>
#include "icom_flags.h"
#include "string_parser.h"
#include "notification.h"

static const char* icomFlagStrings[] = {
  "zero",       // ICOM_FLAG_ZERO
  "prot",       // ICOM_FLAG_PROT
  "timeout",    // ICOM_FLAG_TIMEOUT
  "notify",     // ICOM_FLAG_NOTIFY
  "autonotify", // ICOM_FLAG_AUTONOTIFY
//  "prot,zero", // ICOM_FLAG_ZERO | ICOM_FLAG_PROT TODO: create solution for combining flags
};

static inline icomFlags_t findFlag(const char *flagString){
  if(strcmp(flagString, "default") == 0){
    return 0;
  }

  for(int i=0; i<sizeof(icomFlagStrings)/sizeof(*icomFlagStrings); i++){
    if(strcmp(flagString, icomFlagStrings[i]) == 0){
      return (1<<i);
    }
  }

  return ICOM_FLAG_INVALID;
}

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
  char **flagStrings;
  uint32_t flagCount;
  icomFlags_t flags = 0;

  int ret = parser_initFields(&flagStrings, &flagCount, flagString, ',');
  if(ret != 0){
    _E("Failed to parse flag string");
    return ICOM_FLAG_INVALID;
  }

  /* search for the flag string */
  for(int i=0; i<flagCount; i++){
    flags |= findFlag(flagStrings[i]);
  }

  return flags;
}
