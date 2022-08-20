/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredHeadersUtilities.h>

#include <folly/portability/GTest.h>
#include <string>

namespace proxygen { namespace StructuredHeaders {

class StructuredHeadersUtilitiesTest : public testing::Test {};

TEST_F(StructuredHeadersUtilitiesTest, TestLcalpha) {
  for (uint32_t i = 0; i < 256; i++) {
    uint8_t c = (uint8_t)i;
    if (c >= 'a' && c <= 'z') {
      EXPECT_TRUE(isLcAlpha(c));
    } else {
      EXPECT_FALSE(isLcAlpha(c));
    }
  }
}

TEST_F(StructuredHeadersUtilitiesTest, TestIsValidIdentifierChar) {
  for (uint32_t i = 0; i < 256; i++) {
    uint8_t c = (uint8_t)i;
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
        (c == '_' || c == '-' || c == '*' || c == '/')) {
      EXPECT_TRUE(isValidIdentifierChar(c));
    } else {
      EXPECT_FALSE(isValidIdentifierChar(c));
    }
  }
}

TEST_F(StructuredHeadersUtilitiesTest,
       test_isValidEncodedBinaryContentChar_alphanumeric) {
  EXPECT_TRUE(isValidEncodedBinaryContentChar('a'));
  EXPECT_TRUE(isValidEncodedBinaryContentChar('Z'));
  EXPECT_TRUE(isValidEncodedBinaryContentChar('0'));
  EXPECT_TRUE(isValidEncodedBinaryContentChar('9'));
}

TEST_F(StructuredHeadersUtilitiesTest,
       test_isValidEncodedBinaryContentChar_allowed_symbols) {
  EXPECT_TRUE(isValidEncodedBinaryContentChar('+'));
  EXPECT_TRUE(isValidEncodedBinaryContentChar('/'));
  EXPECT_TRUE(isValidEncodedBinaryContentChar('='));
}

TEST_F(StructuredHeadersUtilitiesTest,
       test_isValidEncodedBinaryContentChar_disallowed_symbols) {
  EXPECT_FALSE(isValidEncodedBinaryContentChar('*'));
  EXPECT_FALSE(isValidEncodedBinaryContentChar('_'));
  EXPECT_FALSE(isValidEncodedBinaryContentChar('-'));
  EXPECT_FALSE(isValidEncodedBinaryContentChar(' '));
}

TEST_F(StructuredHeadersUtilitiesTest, TestIsValidStringCharAllowed) {
  EXPECT_TRUE(isValidStringChar(' '));
  EXPECT_TRUE(isValidStringChar('~'));
  EXPECT_TRUE(isValidStringChar('\\'));
  EXPECT_TRUE(isValidStringChar('\"'));
  EXPECT_TRUE(isValidStringChar('a'));
  EXPECT_TRUE(isValidStringChar('0'));
  EXPECT_TRUE(isValidStringChar('A'));
}

TEST_F(StructuredHeadersUtilitiesTest, TestIsValidStringCharDisallowed) {
  EXPECT_FALSE(isValidStringChar('\0'));
  EXPECT_FALSE(isValidStringChar(0x1F));
  EXPECT_FALSE(isValidStringChar(0x7F));
  EXPECT_FALSE(isValidStringChar('\t'));
}

TEST_F(StructuredHeadersUtilitiesTest, TestIsValidIdentifierAllowed) {
  EXPECT_TRUE(isValidIdentifier("a"));
  EXPECT_TRUE(isValidIdentifier("a_0-*/"));
  EXPECT_TRUE(isValidIdentifier("abc___xyz"));
}

TEST_F(StructuredHeadersUtilitiesTest, TestIsValidIdentifierDisallowed) {
  EXPECT_FALSE(isValidIdentifier("aAAA"));
  EXPECT_FALSE(isValidIdentifier("_aa"));
  EXPECT_FALSE(isValidIdentifier("0abc"));
  EXPECT_FALSE(isValidIdentifier(""));
}

TEST_F(StructuredHeadersUtilitiesTest, TestIsValidStringAllowed) {
  EXPECT_TRUE(isValidString("a cat."));
  EXPECT_TRUE(isValidString("!~)($@^^) g"));
  EXPECT_TRUE(isValidString("\\\"\"\\"));
  EXPECT_TRUE(isValidString(""));
}

TEST_F(StructuredHeadersUtilitiesTest, TestIsValidStringDisallowed) {
  EXPECT_FALSE(isValidString("a\tcat."));
  EXPECT_FALSE(isValidString("\x10 aaaaaaa"));
  EXPECT_FALSE(isValidString("chocolate\x11"));
  EXPECT_FALSE(isValidString("pota\nto"));
}

TEST_F(StructuredHeadersUtilitiesTest, TestGoodBinaryContent) {
  EXPECT_TRUE(isValidEncodedBinaryContent("aGVsbG8="));
  EXPECT_TRUE(isValidEncodedBinaryContent("ZGZzZGZmc2Rm"));
  EXPECT_TRUE(isValidEncodedBinaryContent("ZA=="));
}

TEST_F(StructuredHeadersUtilitiesTest, TestBadBinaryContent) {
  EXPECT_FALSE(isValidEncodedBinaryContent("aGVsbG8"));
  EXPECT_FALSE(isValidEncodedBinaryContent("aGVsb G8="));
  EXPECT_FALSE(isValidEncodedBinaryContent("aGVsbG!8="));
  EXPECT_FALSE(isValidEncodedBinaryContent("=aGVsbG8"));
}

TEST_F(StructuredHeadersUtilitiesTest, Test_DecodeBinaryContent) {
  std::string input1 = "ZnJ1aXQ=";
  std::string input2 = "dG9tYXRv";
  std::string input3 = "ZWdncw==";
  EXPECT_EQ(decodeBase64(input1), "fruit");
  EXPECT_EQ(decodeBase64(input2), "tomato");
  EXPECT_EQ(decodeBase64(input3), "eggs");
}

TEST_F(StructuredHeadersUtilitiesTest, Test_EncodeBinaryContent) {
  std::string input1 = "fruit";
  std::string input2 = "tomato";
  std::string input3 = "eggs";
  EXPECT_EQ(encodeBase64(input1), "ZnJ1aXQ=");
  EXPECT_EQ(encodeBase64(input2), "dG9tYXRv");
  EXPECT_EQ(encodeBase64(input3), "ZWdncw==");
}

TEST_F(StructuredHeadersUtilitiesTest, Test_BinaryContentEmpty) {
  std::string input1 = "";
  std::string input2 = "";
  EXPECT_EQ(encodeBase64(input1), "");
  EXPECT_EQ(decodeBase64(input2), "");
}

TEST_F(StructuredHeadersUtilitiesTest, TestItemTypeMatchesContentGood) {
  StructuredHeaderItem item;
  item.value = std::string("\"potato\"");
  item.tag = StructuredHeaderItem::Type::STRING;
  EXPECT_TRUE(itemTypeMatchesContent(item));

  item.value = std::string("a_800");
  item.tag = StructuredHeaderItem::Type::IDENTIFIER;
  EXPECT_TRUE(itemTypeMatchesContent(item));

  item.tag = StructuredHeaderItem::Type::NONE;
  EXPECT_TRUE(itemTypeMatchesContent(item));

  item.value = std::string("hello");
  item.tag = StructuredHeaderItem::Type::BINARYCONTENT;
  EXPECT_TRUE(itemTypeMatchesContent(item));

  item.value = int64_t(88);
  item.tag = StructuredHeaderItem::Type::INT64;
  EXPECT_TRUE(itemTypeMatchesContent(item));

  item.value = double(88.8);
  item.tag = StructuredHeaderItem::Type::DOUBLE;
  EXPECT_TRUE(itemTypeMatchesContent(item));
}

TEST_F(StructuredHeadersUtilitiesTest, TestItemTypeMatchesContentBad) {
  StructuredHeaderItem item;

  item.value = std::string("hello");
  item.tag = StructuredHeaderItem::Type::DOUBLE;
  EXPECT_FALSE(itemTypeMatchesContent(item));
  item.tag = StructuredHeaderItem::Type::INT64;
  EXPECT_FALSE(itemTypeMatchesContent(item));

  item.value = int64_t(68);
  item.tag = StructuredHeaderItem::Type::DOUBLE;
  EXPECT_FALSE(itemTypeMatchesContent(item));
  item.tag = StructuredHeaderItem::Type::STRING;
  EXPECT_FALSE(itemTypeMatchesContent(item));
  item.tag = StructuredHeaderItem::Type::BINARYCONTENT;
  EXPECT_FALSE(itemTypeMatchesContent(item));
  item.tag = StructuredHeaderItem::Type::IDENTIFIER;
  EXPECT_FALSE(itemTypeMatchesContent(item));

  item.value = double(68.8);
  item.tag = StructuredHeaderItem::Type::INT64;
  EXPECT_FALSE(itemTypeMatchesContent(item));
  item.tag = StructuredHeaderItem::Type::IDENTIFIER;
  EXPECT_FALSE(itemTypeMatchesContent(item));
  item.tag = StructuredHeaderItem::Type::STRING;
  EXPECT_FALSE(itemTypeMatchesContent(item));
  item.tag = StructuredHeaderItem::Type::BINARYCONTENT;
  EXPECT_FALSE(itemTypeMatchesContent(item));
}

}} // namespace proxygen::StructuredHeaders
