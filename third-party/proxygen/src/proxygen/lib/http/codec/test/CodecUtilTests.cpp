/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/CodecUtil.h>

#include <folly/portability/GTest.h>

using std::string;

namespace proxygen { namespace test {

folly::ByteRange input(const char *str) {
  return folly::ByteRange(reinterpret_cast<const uint8_t *>(str), strlen(str));
}

TEST(CodecUtil, validateURL) {
  EXPECT_TRUE(CodecUtil::validateURL(input("/foo"), URLValidateMode::STRICT));
  EXPECT_TRUE(CodecUtil::validateURL(input("/foo\xff"),
                                     URLValidateMode::STRICT_COMPAT));
  EXPECT_FALSE(
      CodecUtil::validateURL(input("/foo\xff"), URLValidateMode::STRICT));
}

TEST(CodecUtil, isalpha) {
  for (uint16_t c = 0; c <= 255; c++) {
    EXPECT_EQ(bool(CodecUtil::isalpha(uint8_t(c))),
              bool(::isalpha(uint8_t(c))));
  }
}

TEST(CodecUtil, validateMethod) {
  EXPECT_TRUE(CodecUtil::validateMethod(input("GET")));
  EXPECT_TRUE(CodecUtil::validateMethod(input("CONNECT-UDP")));
  // TODO:
  // EXPECT_FALSE(CodecUtil::validateMethod(input("CONNECT-")));
  EXPECT_FALSE(CodecUtil::validateMethod(input("-UDP")));
  EXPECT_FALSE(CodecUtil::validateMethod(input("-")));
  EXPECT_TRUE(CodecUtil::validateMethod(input("lowercase")));
}

TEST(CodecUtil, validateScheme) {
  EXPECT_TRUE(CodecUtil::validateScheme(input("http")));
  EXPECT_TRUE(CodecUtil::validateScheme(input("foo")));
  EXPECT_FALSE(CodecUtil::validateScheme(input("h1th3r3")));
}

TEST(CodecUtil, validateHeaderName) {
  EXPECT_TRUE(CodecUtil::validateHeaderName(input("foo"),
                                            CodecUtil::HEADER_NAME_STRICT));
  EXPECT_TRUE(CodecUtil::validateHeaderName(input("foo_bar"),
                                            CodecUtil::HEADER_NAME_STRICT));
  EXPECT_TRUE(CodecUtil::validateHeaderName(input("foo-bar"),
                                            CodecUtil::HEADER_NAME_STRICT));
  EXPECT_FALSE(
      CodecUtil::validateHeaderName(input(""), CodecUtil::HEADER_NAME_STRICT));
  EXPECT_FALSE(CodecUtil::validateHeaderName(input(":foo"),
                                             CodecUtil::HEADER_NAME_STRICT));
  EXPECT_FALSE(CodecUtil::validateHeaderName(input("foo:bar"),
                                             CodecUtil::HEADER_NAME_STRICT));
  EXPECT_FALSE(CodecUtil::validateHeaderName(input("foo\xf0"),
                                             CodecUtil::HEADER_NAME_STRICT));
  EXPECT_FALSE(CodecUtil::validateHeaderName(input("foo\r"),
                                             CodecUtil::HEADER_NAME_STRICT));
  EXPECT_FALSE(CodecUtil::validateHeaderName(input("foo\n"),
                                             CodecUtil::HEADER_NAME_STRICT));
  EXPECT_FALSE(CodecUtil::validateHeaderName(input("foo\r\nfoo"),
                                             CodecUtil::HEADER_NAME_STRICT));

  std::array<char, 19> httpSeparators{'(',
                                      ')',
                                      '<',
                                      '>',
                                      '@',
                                      ',',
                                      ';',
                                      ':',
                                      '\\',
                                      '\"',
                                      '/',
                                      '[',
                                      ']',
                                      '?',
                                      '=',
                                      '{',
                                      '}',
                                      ' ',
                                      '\t'};
  for (auto sep : httpSeparators) {
    auto testHeader = folly::to<std::string>("foo", sep, "bar");
    EXPECT_FALSE(CodecUtil::validateHeaderName(input(testHeader.c_str()),
                                               CodecUtil::HEADER_NAME_STRICT));
    if (sep == ' ' || sep == '/' || sep == '}' || sep == '"') {
      EXPECT_TRUE(CodecUtil::validateHeaderName(
          input(testHeader.c_str()), CodecUtil::HEADER_NAME_STRICT_COMPAT));
    }
  }
}

TEST(CodecUtil, validateHeaderValue) {
  EXPECT_TRUE(CodecUtil::validateHeaderValue(input("abc"), CodecUtil::STRICT));
  string allTheChars;
  allTheChars.reserve(127 - 32);
  for (uint8_t i = 32; i < 127; i++) {
    allTheChars += folly::to<char>(i);
  }
  EXPECT_TRUE(CodecUtil::validateHeaderValue(input(allTheChars.c_str()),
                                             CodecUtil::STRICT));
  // test without leading whitespace
  EXPECT_TRUE(CodecUtil::validateHeaderValue(input(allTheChars.c_str() + 1),
                                             CodecUtil::STRICT));

  // valid lws
  EXPECT_TRUE(
      CodecUtil::validateHeaderValue(input("abc\r\n\tdef"), CodecUtil::STRICT));
  EXPECT_TRUE(CodecUtil::validateHeaderValue(input("abc\r\n \t \t def"),
                                             CodecUtil::STRICT));
  // Invalid lws
  EXPECT_FALSE(CodecUtil::validateHeaderValue(input("abc\r \t \t def"),
                                              CodecUtil::STRICT));
  EXPECT_FALSE(
      CodecUtil::validateHeaderValue(input("abc\r\ndef"), CodecUtil::STRICT));
  // terminating open quote
  EXPECT_TRUE(
      CodecUtil::validateHeaderValue(input("abc\""), CodecUtil::STRICT));
  // open quote
  EXPECT_TRUE(
      CodecUtil::validateHeaderValue(input("abc\"def"), CodecUtil::STRICT));
  // quoted def
  EXPECT_TRUE(
      CodecUtil::validateHeaderValue(input("abc\"def\""), CodecUtil::STRICT));
  // quoted, escaped CRLF
  EXPECT_TRUE(CodecUtil::validateHeaderValue(input("abc\"\\\r\\\n\""),
                                             CodecUtil::COMPLIANT));
  EXPECT_FALSE(CodecUtil::validateHeaderValue(input("abc\"\\\r\\\n\""),
                                              CodecUtil::STRICT));
  EXPECT_TRUE(CodecUtil::validateHeaderValue(input("abc\xff"),
                                             CodecUtil::STRICT_COMPAT));
  EXPECT_FALSE(
      CodecUtil::validateHeaderValue(input("abc\xff"), CodecUtil::STRICT));
  EXPECT_FALSE(
      CodecUtil::validateHeaderValue(input("abc\x7f"), CodecUtil::STRICT));
  // hard-tab OK
  EXPECT_TRUE(
      CodecUtil::validateHeaderValue(input("abc\t"), CodecUtil::STRICT));

  // End on escape
  EXPECT_FALSE(
      CodecUtil::validateHeaderValue(input("\"\\"), CodecUtil::COMPLIANT));
  // End on partial LWS
  EXPECT_FALSE(
      CodecUtil::validateHeaderValue(input("foo\r"), CodecUtil::COMPLIANT));
  EXPECT_FALSE(
      CodecUtil::validateHeaderValue(input("foo\r\n"), CodecUtil::COMPLIANT));

  // leading white space stripped (copied EXPECT_TRUE cases from above and
  // added ws to beginning)
  EXPECT_TRUE(
      CodecUtil::validateHeaderValue(input("\tabc\t"), CodecUtil::STRICT));
  EXPECT_TRUE(CodecUtil::validateHeaderValue(input(" abc\r\n\tdef"),
                                             CodecUtil::STRICT));
  EXPECT_TRUE(CodecUtil::validateHeaderValue(input("\tabc\"\\\r\\\n\""),
                                             CodecUtil::COMPLIANT));
}

TEST(CodecUtil, hasGzipAndDeflate) {
  bool gzip = false;
  bool deflate = false;
  EXPECT_FALSE(CodecUtil::hasGzipAndDeflate("gzip", gzip, deflate));
  EXPECT_TRUE(gzip);
  gzip = false;
  deflate = false;
  EXPECT_FALSE(CodecUtil::hasGzipAndDeflate("deflate", gzip, deflate));
  EXPECT_TRUE(deflate);
  EXPECT_TRUE(CodecUtil::hasGzipAndDeflate("gzip, deflate", gzip, deflate));
  EXPECT_TRUE(CodecUtil::hasGzipAndDeflate("deflate, gzip", gzip, deflate));
  EXPECT_TRUE(
      CodecUtil::hasGzipAndDeflate("foo, gzip, bar, deflate", gzip, deflate));
  EXPECT_FALSE(CodecUtil::hasGzipAndDeflate("zipg, default", gzip, deflate));
  EXPECT_FALSE(
      CodecUtil::hasGzipAndDeflate("gzip; q=.00001, deflate", gzip, deflate));
}

}} // namespace proxygen::test
