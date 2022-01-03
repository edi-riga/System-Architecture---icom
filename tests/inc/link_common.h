#ifndef _LINK_COMMON_H_
#define _LINK_COMMON_H_

void link_common_initialization(const char *icomStr, unsigned testCount);
void link_common_simple(const char *icomTxStr, const char *icomRxStr, uint32_t txBufSize);
void link_common_varied(const char *icomTxStr, const char *icomRxStr, unsigned testCount);
void link_common_fanin(const char *connectStrings[], const char *bindString, unsigned connectCount);
void link_common_complex(const char *connectStrings[], const char *bindStrings[]);

#endif
