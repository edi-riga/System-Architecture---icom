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

TEST(string_parser, five_strings_comma_brackets){
  char **strArray;
  unsigned strCount;
  const char *str = "inproc[0-1],inproc2,inproc[3-4]";
  int ret;

  ret = parser_initStrArray(&strArray, &strCount, str);
  EXPECT_TRUE(0 == ret);
  EXPECT_TRUE(5 == strCount);
  EXPECT_STREQ(strArray[0], "inproc0");
  EXPECT_STREQ(strArray[1], "inproc1");
  EXPECT_STREQ(strArray[2], "inproc2");
  EXPECT_STREQ(strArray[3], "inproc3");
  EXPECT_STREQ(strArray[4], "inproc4");

  parser_deinitStrArray(strArray, strCount);
}

TEST(string_parser, fieldInit0){
  char **fieldArray; uint32_t fieldCount;
  const char *str = NULL;
  int ret;

  ret = parser_initFields(&fieldArray, &fieldCount, str, ':');
  EXPECT_EQ(-1, ret);
  EXPECT_EQ( 0, fieldCount);
  EXPECT_TRUE(NULL == fieldArray);
}

TEST(string_parser, fieldInit1){
  char **fieldArray; uint32_t fieldCount;
  const char *str = "field0";
  int ret;

  ret = parser_initFields(&fieldArray, &fieldCount, str, ':');
  EXPECT_EQ(0, ret);
  EXPECT_EQ(1, fieldCount);
  EXPECT_STREQ("field0", fieldArray[0]);

  parser_deinitFields(fieldArray, fieldCount);
}

TEST(string_parser, fieldInit2){
  char **fieldArray; uint32_t fieldCount;
  const char *str = "field0:field1";
  int ret;

  ret = parser_initFields(&fieldArray, &fieldCount, str, ':');
  EXPECT_EQ(0, ret);
  EXPECT_EQ(2, fieldCount);
  EXPECT_STREQ("field0", fieldArray[0]);
  EXPECT_STREQ("field1", fieldArray[1]);

  parser_deinitFields(fieldArray, fieldCount);
}

TEST(string_parser, fieldInit3){
  char **fieldArray; uint32_t fieldCount;
  const char *str = "field0:field1:field2";
  int ret;

  ret = parser_initFields(&fieldArray, &fieldCount, str, ':');
  EXPECT_EQ(0, ret);
  EXPECT_EQ(3, fieldCount);
  EXPECT_STREQ("field0", fieldArray[0]);
  EXPECT_STREQ("field1", fieldArray[1]);
  EXPECT_STREQ("field2", fieldArray[2]);

  parser_deinitFields(fieldArray, fieldCount);
}

TEST(string_parser, fieldInit4){
  char **fieldArray; uint32_t fieldCount;
  const char *str = "field0:field1:field2:field3";
  int ret;

  ret = parser_initFields(&fieldArray, &fieldCount, str, ':');
  EXPECT_EQ(0, ret);
  EXPECT_EQ(4, fieldCount);
  EXPECT_STREQ("field0", fieldArray[0]);
  EXPECT_STREQ("field1", fieldArray[1]);
  EXPECT_STREQ("field2", fieldArray[2]);
  EXPECT_STREQ("field3", fieldArray[3]);

  parser_deinitFields(fieldArray, fieldCount);
}
