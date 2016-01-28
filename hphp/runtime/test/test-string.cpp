/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <gtest/gtest.h>

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/ext/string/ext_string.h"

namespace HPHP {

#define VS(str1, str2) EXPECT_TRUE(!strcmp((str1), (str2)))

TEST(String, Constructors) {
  // constructors
  VS(String(15).c_str(), "15");
  VS(String(-15).c_str(), "-15");
  VS(String(int64_t(12345678912345678LL)).c_str(), "12345678912345678");
  VS(String(int64_t(-12345678912345678LL)).c_str(), "-12345678912345678");
  VS(String(5.603).c_str(), "5.603");
  VS(String("test").c_str(), "test");
  VS(String(String("test")).c_str(), "test");
}

TEST(String, Informational) {
  EXPECT_TRUE(String().isNull());
  EXPECT_TRUE(!String("").isNull());
  EXPECT_TRUE(String().empty());
  EXPECT_TRUE(String("").empty());
  EXPECT_TRUE(!String("test").empty());
  EXPECT_TRUE(String().size() == 0);
  EXPECT_TRUE(String().length() == 0);
  EXPECT_TRUE(String("").size() == 0);
  EXPECT_TRUE(String("").length() == 0);
  EXPECT_TRUE(String("test").size() == 4);
  EXPECT_TRUE(String("test").length() == 4);
  EXPECT_TRUE(!String("2test").isNumeric());
  EXPECT_TRUE(!String("2test").isInteger());
  EXPECT_TRUE(!String("test").isNumeric());
  EXPECT_TRUE(!String("test").isInteger());
  EXPECT_TRUE(String("23").isNumeric());
  EXPECT_TRUE(String("23").isInteger());
  EXPECT_TRUE(String("23.3").isNumeric());
  EXPECT_TRUE(!String("23.3").isInteger());
}

TEST(String, Operators) {
  String s;
  s = "test1";                   VS(s.c_str(), "test1");
  s = String("test2");           VS(s.c_str(), "test2");
  s = Variant("test3");          VS(s.c_str(), "test3");
  s = String("a") + "b";         VS(s.c_str(), "ab");
  s = String("c") + String("d"); VS(s.c_str(), "cd");
  s += "efg";                    VS(s.c_str(), "cdefg");
  s += String("hij");            VS(s.c_str(), "cdefghij");
}

TEST(String, Manipulations) {
  String s = HHVM_FN(strtolower)("Test");
  VS(s.c_str(), "test");
}

TEST(String, Conversions) {
  EXPECT_TRUE(!String().toBoolean());
  EXPECT_TRUE(String("123").toBoolean());
  EXPECT_TRUE(String("123").toByte() == 123);
  EXPECT_TRUE(String("32767").toInt16() == 32767);
  EXPECT_TRUE(String("1234567890").toInt32() == 1234567890);
  EXPECT_TRUE(String("123456789012345678").toInt64() == 123456789012345678LL);
  EXPECT_TRUE(String("123.45").toDouble() == 123.45);
}

TEST(String, Compare) {
  EXPECT_TRUE(String("abc").compare("abc") == 0);
  EXPECT_TRUE(String("abc").compare("bbc") < 0);
  EXPECT_TRUE(String("bbc").compare("abc") > 0);
  EXPECT_TRUE(String("abc").compare(String("abc")) == 0);
  EXPECT_TRUE(String("abc").compare(String("bbc")) < 0);
  EXPECT_TRUE(String("bbc").compare(String("abc")) > 0);

  EXPECT_TRUE(String("abc").compare("abcd") < 0);
  EXPECT_TRUE(String("abcd").compare("abc") > 0);
  EXPECT_TRUE(String("abc").compare(String("abcd")) < 0);
  EXPECT_TRUE(String("abcd").compare(String("abc")) > 0);

  // check Strings with embedded nulls
  std::string str1("a\0bc", 4);
  std::string str2("a\0bb", 4);
  EXPECT_TRUE(String(str1).compare(String(str1)) == 0);
  EXPECT_TRUE(String(str1).compare(String(str2)) > 0);
  EXPECT_TRUE(String(str2).compare(String(str1)) < 0);
}

}
