/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/utils/UtilInl.h>

#include <cctype>

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

TEST(UtilTest, clamped) {
  EXPECT_EQ(clamped_downcast<uint8_t>(uint64_t(255)),
            std::numeric_limits<uint8_t>::max());
  EXPECT_EQ(clamped_downcast<uint8_t>(uint64_t(256)),
            std::numeric_limits<uint8_t>::max());
  EXPECT_EQ(clamped_downcast<uint8_t>(std::numeric_limits<uint64_t>::max()),
            std::numeric_limits<uint8_t>::max());

  EXPECT_EQ(clamped_downcast<uint16_t>(uint64_t(65535)),
            std::numeric_limits<uint16_t>::max());
  EXPECT_EQ(clamped_downcast<uint16_t>(uint64_t(65536)),
            std::numeric_limits<uint16_t>::max());
  EXPECT_EQ(clamped_downcast<uint16_t>(std::numeric_limits<uint64_t>::max()),
            std::numeric_limits<uint16_t>::max());

  EXPECT_EQ(clamped_downcast<uint32_t>(
                uint64_t(std::numeric_limits<uint32_t>::max())),
            std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(clamped_downcast<uint32_t>(
                uint64_t(std::numeric_limits<uint32_t>::max()) + 1),
            std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(clamped_downcast<uint32_t>(std::numeric_limits<uint64_t>::max()),
            std::numeric_limits<uint32_t>::max());
}

TEST(UtilTest, isAlpha) {
  // Test is only valid in the "C" locale.
  for (uint16_t c = 0; c <= 255; c++) {
    EXPECT_EQ(bool(isAlpha(uint8_t(c))), bool(std::isalpha(uint8_t(c))));
  }
}
