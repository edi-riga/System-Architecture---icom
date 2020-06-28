#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "icom.h"

#define _I(fmt, args...)   printf(fmt "\n", ##args)

int main(void){
    icom_t *icomPush, *icomPull;

    _I("Initializing icom API");
    icom_init();

    _I("Initializing inproc PUSH socket");
    icomPush = icom_initPush("inproc://tmp", 1024, 2);

    _I("Initializing inproc PULL socket");
    icomPull = icom_initPull("inproc://tmp", 1024);

    _I("Releasing inproc PUSH socket");
    icom_deinitPush(icomPush);

    _I("Releasing inproc PULL socket");
    icom_deinitPull(icomPull);

    _I("Initializing IP PUSH socket");
    icomPush = icom_initPush("127.0.0.1://tmp", 1024, 2);

    _I("Initializing IP PULL socket");
    icomPull = icom_initPull("127.0.0.1://tmp", 1024);

    _I("Releasing IP PUSH socket");
    icom_deinitPush(icomPush);

    _I("Releasing IP PULL socket");
    icom_deinitPull(icomPull);

    _I("Deinitializing icom API");
    icom_release();

    return 0;
}
