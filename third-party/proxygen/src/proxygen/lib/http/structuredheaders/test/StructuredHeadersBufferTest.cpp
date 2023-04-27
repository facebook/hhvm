/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredHeadersBuffer.h>

#include <folly/Conv.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersConstants.h>
#include <string>

namespace proxygen {

class StructuredHeadersBufferTest : public testing::Test {};

TEST_F(StructuredHeadersBufferTest, TestBinaryContent) {
  std::string input = "*bWF4aW0gaXMgdGhlIGJlc3Q=*";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::BINARYCONTENT);
  EXPECT_EQ(output, std::string("maxim is the best"));
}

TEST_F(StructuredHeadersBufferTest, TestBinaryContentIllegalCharacters) {
  std::string input = "*()645\t  this is not a b64 encoded string ((({]}}}))*";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestBinaryContentNoEndingAsterisk) {
  std::string input = "*seattle";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestBinaryContentEmpty) {
  std::string input = "**";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::BINARYCONTENT);
  EXPECT_EQ(output, std::string(""));
}

TEST_F(StructuredHeadersBufferTest, TestIdentifier) {
  std::string input = "abcdefg";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseIdentifier(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::IDENTIFIER);
  EXPECT_EQ(output, std::string("abcdefg"));
}

TEST_F(StructuredHeadersBufferTest, TestIdentifierAllLegalCharacters) {
  std::string input = "a0_-*/";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseIdentifier(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::IDENTIFIER);
  EXPECT_EQ(output, std::string("a0_-*/"));
}

TEST_F(StructuredHeadersBufferTest, TestIdentifierBeginningUnderscore) {
  std::string input = "_af09d____****";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseIdentifier(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestString) {
  std::string input = "\"fsdfsdf\"sdfsdf\"";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(output, std::string("fsdfsdf"));
}

TEST_F(StructuredHeadersBufferTest, TestStringEscapedQuote) {
  std::string input = "\"abc\\\"def\"";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(output, std::string("abc\"def"));
}

TEST_F(StructuredHeadersBufferTest, TestStringEscapedBackslash) {
  std::string input = "\"abc\\\\def\"";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(output, std::string("abc\\def"));
}

TEST_F(StructuredHeadersBufferTest, TestStringStrayBackslash) {
  std::string input = "\"abc\\def\"";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestStringInvalidCharacter) {
  std::string input = "\"abcdefg\thij\"";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestStringParsingRepeated) {
  std::string input = "\"proxy\"\"gen\"";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(output, std::string("proxy"));

  err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(output, std::string("gen"));
}

TEST_F(StructuredHeadersBufferTest, TestInteger) {
  std::string input = "843593";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(output, int64_t(843593));
}

TEST_F(StructuredHeadersBufferTest, TestIntegerTwoNegatives) {
  std::string input = "--843593";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestIntegerEmptyAfterNegative) {
  std::string input = "-";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestIntegerNegative) {
  std::string input = "-843593";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(output, int64_t(-843593));
}

TEST_F(StructuredHeadersBufferTest, TestIntegerOverflow) {
  std::string input = "9223372036854775808";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestIntegerHighBorderline) {
  std::string input = "9223372036854775807";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(output, std::numeric_limits<int64_t>::max());
}

TEST_F(StructuredHeadersBufferTest, TestIntegerLowBorderline) {
  std::string input = "-9223372036854775808";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(output, std::numeric_limits<int64_t>::min());
}

TEST_F(StructuredHeadersBufferTest, TestIntegerUnderflow) {
  std::string input = "-9223372036854775809";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestBool) {
  for (auto i = 0; i < 2; i++) {
    std::string input = folly::to<std::string>("?", i);
    StructuredHeadersBuffer shd(input);
    StructuredHeaderItem output;
    auto err = shd.parseItem(output);
    EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
    EXPECT_EQ(output.tag, StructuredHeaderItem::Type::BOOLEAN);
    bool expected = i;
    EXPECT_EQ(output.get<bool>(), expected);
  }
}

TEST_F(StructuredHeadersBufferTest, TestBoolInvalidChars) {
  std::string input = "?2";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::INVALID_CHARACTER);
}

TEST_F(StructuredHeadersBufferTest, TestBoolWrongLength) {
  std::vector<std::string> inputs{"?", "?10"};
  for (auto& input : inputs) {
    StructuredHeadersBuffer shd(input);
    StructuredHeaderItem output;
    auto err = shd.parseItem(output);
    EXPECT_EQ(err,
              (input.length() > 2
                   ? StructuredHeaders::DecodeError::VALUE_TOO_LONG
                   : StructuredHeaders::DecodeError::UNEXPECTED_END_OF_BUFFER));
  }
}

TEST_F(StructuredHeadersBufferTest, TestBool2) {

  std::string input = "?2";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::INVALID_CHARACTER);
}

TEST_F(StructuredHeadersBufferTest, TestFloat) {
  std::string input = "3.1415926536";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::DOUBLE);
  EXPECT_EQ(output, 3.1415926536);
}

TEST_F(StructuredHeadersBufferTest, TestFloatPrecedingWhitespace) {
  std::string input = "         \t\t    66000.5645";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::DOUBLE);
  EXPECT_EQ(output, 66000.5645);
}

TEST_F(StructuredHeadersBufferTest, TestFloatNoDigitPrecedingDecimal) {
  std::string input = ".1415926536";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestIntegerTooManyChars) {
  std::string input = "10000000000000000000"; // has 20 characters
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestFloatTooManyChars) {
  std::string input = "111111111.1111111"; // has 17 characters
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestFloatBorderlineNumChars) {
  std::string input = "111111111.111111"; // has 16 characters
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::DOUBLE);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestFloatEndsWithDecimal) {
  std::string input = "100.";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  auto err = shd.parseItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersBufferTest, TestConsumeComma) {
  std::string input = ",5345346";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  shd.removeSymbol(",", true);
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(output, int64_t(5345346));
}

TEST_F(StructuredHeadersBufferTest, TestConsumeEquals) {
  std::string input = "=456346.646";
  StructuredHeadersBuffer shd(input);
  StructuredHeaderItem output;
  shd.removeSymbol("=", true);
  auto err = shd.parseItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::DOUBLE);
  EXPECT_EQ(output, 456346.646);
}

TEST_F(StructuredHeadersBufferTest, TestConsumeMessy) {
  std::string input = "asfgsdfg,asfgsdfg,";
  StructuredHeadersBuffer shd(input);
  for (int i = 0; i < 2; i++) {
    StructuredHeaderItem output;
    auto err = shd.parseIdentifier(output);
    EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
    EXPECT_EQ(output.tag, StructuredHeaderItem::Type::IDENTIFIER);
    EXPECT_EQ(output, std::string("asfgsdfg"));
    shd.removeSymbol(",", true);
  }
}

TEST_F(StructuredHeadersBufferTest, TestInequalityOperator) {
  StructuredHeaderItem integerItem;
  integerItem.value = int64_t(999);

  StructuredHeaderItem doubleItem;
  doubleItem.value = 11.43;

  StructuredHeaderItem stringItem;
  stringItem.value = std::string("hi");

  EXPECT_NE(integerItem, int64_t(998));
  EXPECT_NE(doubleItem, double(11.44));
  EXPECT_NE(stringItem, std::string("bye"));
}

} // namespace proxygen
