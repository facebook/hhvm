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

#include "hphp/util/text-util.h"
#include <gtest/gtest.h>

namespace HPHP {

using std::string;
using std::vector;

class TestTextUtil : public testing::Test {
};

TEST_F(TestTextUtil, EndsWith) {
  EXPECT_FALSE(TextUtil::EndsWith("", 'a'));
  EXPECT_FALSE(TextUtil::EndsWith("b", 'a'));
  EXPECT_FALSE(TextUtil::EndsWith("b", 'a'));
  EXPECT_TRUE(TextUtil::EndsWith("ab", 'b'));
  EXPECT_FALSE(TextUtil::EndsWith("abc", 'b'));
  EXPECT_FALSE(TextUtil::EndsWith("bbbbbbbbbbbba", 'b'));
}

TEST_F(TestTextUtil, StripTrailing) {
  EXPECT_EQ(TextUtil::StripTrailing("abcd", 'd'), "abc");
  EXPECT_EQ(TextUtil::StripTrailing("dddd", 'd'), "");
}

TEST_F(TestTextUtil, BeginsWith) {
  EXPECT_TRUE(TextUtil::BeginsWith("12345", "123"));
  EXPECT_TRUE(TextUtil::BeginsWith("12345", "12"));
  EXPECT_TRUE(TextUtil::BeginsWith("12345", "1"));
  EXPECT_TRUE(TextUtil::BeginsWith("12345", ""));

  EXPECT_TRUE(TextUtil::BeginsWith("1", "1"));
  EXPECT_TRUE(TextUtil::BeginsWith("12", "1"));
  EXPECT_TRUE(TextUtil::BeginsWith("123", "1"));

  EXPECT_FALSE(TextUtil::BeginsWith("0123", "1"));

  EXPECT_TRUE(TextUtil::BeginsWith("", ""));

  EXPECT_FALSE(TextUtil::BeginsWith("", "abcd"));
}

TEST_F(TestTextUtil, StripCommonStart) {
  EXPECT_EQ(TextUtil::StripCommonStart("abcde", "abc"), "de");
  EXPECT_EQ(TextUtil::StripCommonStart("abcde", "xyz"), "abcde");
  EXPECT_EQ(TextUtil::StripCommonStart("abcde", ""), "abcde");
  EXPECT_EQ(TextUtil::StripCommonStart("", "123"), "");
  EXPECT_EQ(TextUtil::StripCommonStart("", ""), "");
  EXPECT_EQ(TextUtil::StripCommonStart("123", ""), "123");
}

TEST_F(TestTextUtil, GetPathList) {
  vector<string> out;

  out = TextUtil::MakePathList("/some/path/to/foo");
  ASSERT_EQ(out.size(), 3);
  EXPECT_EQ(out[0], "/some");
  EXPECT_EQ(out[1], "/some/path");
  EXPECT_EQ(out[2], "/some/path/to");

  out = TextUtil::MakePathList("/justonebiglongname");
  ASSERT_EQ(out.size(), 0);

  out = TextUtil::MakePathList("trailingslashonly/");
  ASSERT_EQ(out.size(), 1);
  EXPECT_EQ(out[0], "trailingslashonly");

  out = TextUtil::MakePathList("middle/slash");
  ASSERT_EQ(out.size(), 1);
  EXPECT_EQ(out[0], "middle");

  out = TextUtil::MakePathList("noslashesatall");
  ASSERT_EQ(out.size(), 0);

  out = TextUtil::MakePathList("");
  ASSERT_EQ(out.size(), 0);

  out = TextUtil::MakePathList("foo/bar/index.php");
  ASSERT_EQ(out.size(), 2);
  EXPECT_EQ(out[0], "foo");
  EXPECT_EQ(out[1], "foo/bar");
}

}  // namespace HPHP
