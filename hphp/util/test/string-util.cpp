/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <string>
#include <vector>

#include "hphp/util/string-util.h"
#include "gtest/gtest.h"

namespace HPHP {

using std::string;
using std::vector;

class TestStringUtil : public testing::Test {
};

TEST_F(TestStringUtil, EndsWith) {
  EXPECT_FALSE(StringUtil::EndsWith("", 'a'));
  EXPECT_FALSE(StringUtil::EndsWith("b", 'a'));
  EXPECT_FALSE(StringUtil::EndsWith("b", 'a'));
  EXPECT_TRUE(StringUtil::EndsWith("ab", 'b'));
  EXPECT_FALSE(StringUtil::EndsWith("abc", 'b'));
  EXPECT_FALSE(StringUtil::EndsWith("bbbbbbbbbbbba", 'b'));
}

TEST_F(TestStringUtil, StripTrailing) {
  EXPECT_EQ(StringUtil::StripTrailing("abcd", 'd'), "abc");
  EXPECT_EQ(StringUtil::StripTrailing("dddd", 'd'), "");
}

TEST_F(TestStringUtil, BeginsWith) {
  EXPECT_TRUE(StringUtil::BeginsWith("12345", "123"));
  EXPECT_TRUE(StringUtil::BeginsWith("12345", "12"));
  EXPECT_TRUE(StringUtil::BeginsWith("12345", "1"));
  EXPECT_FALSE(StringUtil::BeginsWith("12345", ""));

  EXPECT_TRUE(StringUtil::BeginsWith("1", "1"));
  EXPECT_TRUE(StringUtil::BeginsWith("12", "1"));
  EXPECT_TRUE(StringUtil::BeginsWith("123", "1"));

  EXPECT_FALSE(StringUtil::BeginsWith("0123", "1"));

  EXPECT_FALSE(StringUtil::BeginsWith("", ""));

  EXPECT_FALSE(StringUtil::BeginsWith("", "abcd"));
}

TEST_F(TestStringUtil, StripCommonStart) {
  EXPECT_EQ(StringUtil::StripCommonStart("abcde", "abc"), "de");
  EXPECT_EQ(StringUtil::StripCommonStart("abcde", "xyz"), "abcde");
  EXPECT_EQ(StringUtil::StripCommonStart("abcde", ""), "abcde");
  EXPECT_EQ(StringUtil::StripCommonStart("", "123"), "");
  EXPECT_EQ(StringUtil::StripCommonStart("", ""), "");
}

TEST_F(TestStringUtil, GetPathList) {
  vector<string> out;

  out = StringUtil::MakePathList("/some/path/to/foo");

  ASSERT_EQ(out.size(), 4);
  EXPECT_EQ(out[0], "/some");
  EXPECT_EQ(out[1], "/some/path");
  EXPECT_EQ(out[2], "/some/path/to");
  EXPECT_EQ(out[3], "/some/path/to/foo");

  out = StringUtil::MakePathList("/justonebiglongname");

  ASSERT_EQ(out.size(), 1);
  EXPECT_EQ(out[0], "/justonebiglongname");

  out = StringUtil::MakePathList("trailingslashonly/");
  ASSERT_EQ(out.size(), 0);

  out = StringUtil::MakePathList("middle/slash");
  ASSERT_EQ(out.size(), 0);

  out = StringUtil::MakePathList("noslashesatall");
  ASSERT_EQ(out.size(), 0);

  out = StringUtil::MakePathList("");
  ASSERT_EQ(out.size(), 0);
}

}  // namespace HPHP
