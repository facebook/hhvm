/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/utils/UtilInl.h>

using namespace proxygen;

TEST(UtilTest, CaseInsensitiveEqual) {
  ASSERT_TRUE(caseInsensitiveEqual("foo", "FOO"));
  ASSERT_TRUE(caseInsensitiveEqual(std::string("foo"), "FOO"));
  ASSERT_FALSE(caseInsensitiveEqual(std::string("foo"), "FOO2"));
  ASSERT_FALSE(caseInsensitiveEqual("fo", "FOO"));
  ASSERT_FALSE(caseInsensitiveEqual("FO", "FOO"));
}

TEST(UtilTest, findLastOf) {
  folly::StringPiece p1("");
  folly::StringPiece p2(".");
  folly::StringPiece p3("..");
  folly::StringPiece p4("abc");
  folly::StringPiece p5("abc.def");

  EXPECT_EQ(findLastOf(p1, '.'), std::string::npos);
  EXPECT_EQ(findLastOf(p2, '.'), 0);
  EXPECT_EQ(findLastOf(p3, '.'), 1);
  EXPECT_EQ(findLastOf(p4, '.'), std::string::npos);
  EXPECT_EQ(findLastOf(p5, '.'), 3);
}

folly::ByteRange input(const char *str) {
  return folly::ByteRange(reinterpret_cast<const uint8_t *>(str), strlen(str));
}

TEST(UtilTest, validateURL) {
  EXPECT_TRUE(validateURL(input("/foo\xff"), URLValidateMode::STRICT_COMPAT));
  EXPECT_FALSE(validateURL(input("/foo\xff"), URLValidateMode::STRICT));
}
