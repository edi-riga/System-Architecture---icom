#ifndef _LINK_COMMON_H_
#define _LINK_COMMON_H_

void link_common_initialization(const char *icomStr, unsigned testCount);
void link_common_simple(const char *icomTxStr, const char *icomRxStr, uint32_t txBufSize);
void link_common_varied(const char *icomTxStr, const char *icomRxStr, unsigned testCount);
void link_common_timeout_rx(const char *icomRxStr, const char *icomTxStr);
void link_common_timeout_tx(const char *icomRxStr, const char *icomTxStr);
void link_common_topology(std::vector<const char*> icomRxStr, std::vector<const char*> icomTxStr, uint32_t transfers);

#endif
