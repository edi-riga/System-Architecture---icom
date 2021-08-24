#include "gtest/gtest.h"
extern "C" {
  #include "string_parser.h"
}

TEST(string_parser, null_string){
  char **strArray;
  unsigned strCount;
  char *str = NULL;
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(ret == -1);
  EXPECT_TRUE(strArray == NULL);
  EXPECT_TRUE(strCount == 0);
}

TEST(string_parser, single_string){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(1 == strCount);
  EXPECT_STREQ(strArray[0], "inproc");

  parser_deinitStrArray(strArray, strCount);
}

TEST(string_parser, single_string_brackets){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc[0]";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(1 == strCount);
  EXPECT_STREQ(strArray[0], "inproc0");

  parser_deinitStrArray(strArray, strCount);
}


TEST(string_parser, two_strings_comma){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc0,inproc1";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(0 == strcmp(strArray[0], "inproc0"));
  EXPECT_TRUE(0 == strcmp(strArray[1], "inproc1"));
  EXPECT_TRUE(2 == strCount);

  parser_deinitStrArray(strArray, strCount);
}

TEST(string_parser, two_strings_brackets){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc[0-1]";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(2 == strCount);
  EXPECT_STREQ(strArray[0], "inproc0");
  EXPECT_STREQ(strArray[1], "inproc1");

  parser_deinitStrArray(strArray, strCount);
}

TEST(string_parser, three_strings_comma_brackets){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc0,inproc[1-2]";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(3 == strCount);
  EXPECT_STREQ(strArray[0], "inproc0");
  EXPECT_STREQ(strArray[1], "inproc1");
  EXPECT_STREQ(strArray[2], "inproc2");

  parser_deinitStrArray(strArray, strCount);
}

TEST(string_parser, three_strings_brackets_comma){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc[0-1],inproc2";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(3 == strCount);
  EXPECT_STREQ(strArray[0], "inproc0");
  EXPECT_STREQ(strArray[1], "inproc1");
  EXPECT_STREQ(strArray[2], "inproc2");

  parser_deinitStrArray(strArray, strCount);
}

TEST(string_parser, four_strings_brackets_comma){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc[0-1],inproc2,inproc3";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(4 == strCount);
  EXPECT_STREQ(strArray[0], "inproc0");
  EXPECT_STREQ(strArray[1], "inproc1");
  EXPECT_STREQ(strArray[2], "inproc2");
  EXPECT_STREQ(strArray[3], "inproc3");

  parser_deinitStrArray(strArray, strCount);
}

TEST(string_parser, four_strings_comma_brackets){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc0,inproc1,inproc[2-3]";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(4 == strCount);
  EXPECT_STREQ(strArray[0], "inproc0");
  EXPECT_STREQ(strArray[1], "inproc1");
  EXPECT_STREQ(strArray[2], "inproc2");
  EXPECT_STREQ(strArray[3], "inproc3");

  parser_deinitStrArray(strArray, strCount);
}
