#include "gtest/gtest.h"
extern "C" {
  #include "icom_status.h"
  #include "icom_config.h"
}

TEST(icom_config, timeout_rcv_usec_set){
  icomStatus_t status;
  uint64_t timeSet_us = 777777;
  uint64_t timeGet_us = 0xdeadbeef;

  status = icom_setDefaultConfig(TIMEOUT_RCV_USEC, &timeSet_us);
  EXPECT_TRUE(status == ICOM_SUCCESS);

  status = icom_getDefaultConfig(TIMEOUT_RCV_USEC, &timeGet_us);
  EXPECT_TRUE(status == ICOM_SUCCESS);
  EXPECT_TRUE(timeSet_us == timeGet_us);
}

TEST(icom_config, timeout_snd_usec_set){
  icomStatus_t status;
  uint64_t timeSet_us = 777777;
  uint64_t timeGet_us = 0xdeadbeef;

  status = icom_setDefaultConfig(TIMEOUT_SND_USEC, &timeSet_us);
  EXPECT_TRUE(status == ICOM_SUCCESS);

  status = icom_getDefaultConfig(TIMEOUT_SND_USEC, &timeGet_us);
  EXPECT_TRUE(status == ICOM_SUCCESS);
  EXPECT_TRUE(timeSet_us == timeGet_us);
}

