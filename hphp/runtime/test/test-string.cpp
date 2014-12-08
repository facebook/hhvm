/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

TEST(DataTypes, String) {
  // compare
  {
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

}
