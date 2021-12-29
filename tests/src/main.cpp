#include <stdio.h>
#include <time.h>
#include "gtest/gtest.h"

int main(int argc, char *argv[]){
  /* initialize random number generator */
  srand(time(NULL));

  /* parse command line arguments and init */
  testing::InitGoogleTest(&argc, argv);

  /* run google tests APP */
  return RUN_ALL_TESTS();
}
