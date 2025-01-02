/* @author: Daniels Jānis Justs */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "string_parser.h"
#include "notification.h"

static inline char* strchrnul_custom(const char *ptr, char delimiter) {
  while(*ptr != delimiter && *ptr != '\0') ptr++;
  return (char*)ptr;
}


unsigned parser_getConnectionCount(const char *ptrStart){
  char *ptrStop;
  int   strCount = 0;
  int   idFrom, idTo, ret;

  do{
    // parse entry
    ptrStop = strchrnul_custom(ptrStart, ',');
    char *tmp = strndup(ptrStart, ptrStop - ptrStart);
    ret = sscanf(tmp, "%*[^[][%d-%d]", &idFrom, &idTo);
    free(tmp);

    // update count
    if(ret == 2)
      strCount += idTo - idFrom;
    strCount++;

    // set up for next iteration
    ptrStart = ptrStop + 1;
  }while(*ptrStop != '\0');

  return strCount;
}


int parser_initStrArray(char ***strArray, unsigned *strCount, const char *ptrStart){

  char *ptrStop, *idStr;
  int strCurrent = 0;

  // check if no or empty string has been passed
  if(ptrStart == NULL || ptrStart[0] == '\0'){
    *strArray = NULL;
    *strCount = 0;
    return -1;
  }

  // allocate memories
  *strCount = parser_getConnectionCount(ptrStart);
  *strArray = (char**)malloc((*strCount)*sizeof(char*));
  if(*strArray == NULL){
    _E("Failed to allocate memory");
    goto error_malloc;
    //return -1;
  }

  do{
    // retrieve candidate string
    ptrStop = strchrnul_custom(ptrStart, ',');
    if(strchr(ptrStart, '[') && ptrStop > strchr(ptrStart, '[')){
      while(strchr(ptrStart, ']') > ptrStop){
        ptrStop = strchrnul_custom(ptrStop+1, ',');
      }
    }

    char *candidate = strndup(ptrStart, ptrStop - ptrStart);
    if(!candidate){
      _E("Failed to allocate memory");
      free(strArray);
      return -1;
    }

    // parse the candidate string
    char *rangeStart = strchr(candidate, '[');

    // simple non-range option
    if(rangeStart == NULL){
      (*strArray)[strCurrent] = candidate;
      strCurrent++;
      ptrStart = ptrStop + 1;
      continue;
    }

    // a more complex range-based option
    else {

      sscanf(candidate, "%m[^[]", &idStr);

      // '-', dash-based range
      if(strchr(rangeStart, '-')){

        int idFrom, idTo;
        sscanf(rangeStart, "[%d-%d]", &idFrom, &idTo);

        for(int i = idFrom; i <= idTo; i++){
          unsigned size = snprintf(NULL, 0, "%s%d", idStr, i) + 1;
          (*strArray)[strCurrent] = malloc(size*sizeof(char));
          if((*strArray)[strCurrent] == NULL){
            _E("Failed to allocate memory");
            goto error_malloc_member;
          }
          sprintf((*strArray)[strCurrent], "%s%d", idStr, i);
          strCurrent++;
        }

      }
      // ',', comma-based range
      else if(strchr(rangeStart, ',')){

        char *tok = strtok(rangeStart+1, ",");
        while(tok){
          unsigned size = snprintf(NULL, 0, "%s%d", idStr, atoi(tok)) + 1;
          (*strArray)[strCurrent] = malloc(size*sizeof(char));
          if((*strArray)[strCurrent] == NULL){
            _E("Failed to allocate memory");
            goto error_malloc_member;
          }
          sprintf((*strArray)[strCurrent], "%s%d", idStr, atoi(tok));
          strCurrent++;

          tok = strtok(NULL, ",");
        }

      }
      // range syntax, but with a single member
      else{

        int id;
        sscanf(rangeStart, "[%d]", &id);

        unsigned size = snprintf(NULL, 0, "%s%d", idStr, id) + 1;
        (*strArray)[strCurrent] = malloc(size*sizeof(char));
        if((*strArray)[strCurrent] == NULL){
          _E("Failed to allocate memory");
          goto error_malloc_member;
        }
        sprintf((*strArray)[strCurrent], "%s%d", idStr, id);
        strCurrent++;

      }
    }

    free(candidate);
    free(idStr);

    // set up for next iteration
    ptrStart = ptrStop + 1;
  } while(*ptrStop != '\0');

  return 0;


error_malloc_member:
  for(;strCurrent > 0; strCurrent--){
    free((*strArray)[strCurrent]);
  }

error_malloc:
  free(*strArray);
  *strCount = 0;

  return -1;
}

void parser_deinitStrArray(char **strArray, unsigned strCount){
  for(int i = 0; i < strCount; i++){
    free(strArray[i]);
  }
  free(strArray);
}


int parser_initFields(char ***fieldArray, uint32_t *fieldCount, const char *ptrStart, char separator){
  char *ptrStop;
  int i;

  /* check if no string has been passed */
  if(ptrStart == NULL){
    *fieldArray = NULL;
    *fieldCount = 0;
    return -1;
  }

  /* check if connection string format is correct */
  ptrStop = (char*)ptrStart;
  for(*fieldCount=1; *ptrStop; ptrStop++){
    if(*ptrStop == separator){
      (*fieldCount)++;
    }
  }

  /* allocate array for field array */
  *fieldArray = (char**)malloc((*fieldCount)*sizeof(char*));
  if(*fieldArray == NULL){
    _E("Failed to allocate memory");
    return -1;
  }

  /* duplicate field string array */
  ptrStop = (char*)ptrStart;
  i = 0;
  do{
    ptrStop = strchrnul_custom(ptrStart, separator);
    (*fieldArray)[i] = strndup(ptrStart, ptrStop - ptrStart);
    if(!(*fieldArray)[i]){
      _E("Failed to allocate memory");
      goto failure_duplicate;
    }

    i++;
    ptrStart = ptrStop + 1;
  } while(*ptrStop != '\0');

  return 0;


failure_duplicate:
  for(;i>=0;i--){
    free((*fieldArray)[i]);
  }

  free(*fieldArray);
  return -1;
}


void parser_deinitFields(char **fieldArray, uint32_t fieldCount){
  for(int i=0; i<fieldCount; i++){
    free(fieldArray[i]);
  }

  free(fieldArray);
}
