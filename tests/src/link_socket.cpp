#include "gtest/gtest.h"
extern "C" {
  #include "icom.h"
}

TEST(link_socket, init_bind){
  icom_t *icomBind;

  icomBind = icom_init("*:8889", ICOM_TYPE_SOCKET_BIND, 0);
  EXPECT_FALSE(ICOM_IS_ERR(icomBind));

  icom_deinit(icomBind);
}


//TEST(link_socket, init_connect_with_server){
//  icom_t *icomConnect, *icomBind;
//
//  icomBind = icom_init("*:8889", ICOM_TYPE_SOCKET_BIND, 0);
//  EXPECT_FALSE(ICOM_IS_ERR(icomBind));
//
//  icomConnect = icom_init("127.0.0.1:8889", ICOM_TYPE_SOCKET_CONNECT, 0);
//  EXPECT_TRUE(ICOM_IS_ERR(icomConnect));
//
//  icom_deinit(icomConnect);
//  icom_deinit(icomBind);
//}
