/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <boost/algorithm/string/trim.hpp>
#include <folly/portability/GTest.h>
#include <glog/logging.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersDecoder.h>
#include <proxygen/lib/http/structuredheaders/StructuredHeadersEncoder.h>

namespace proxygen {

/*
 * These test cases were obtained from
 * https://github.com/httpwg/structured-header-tests/commits/master, with a
 * commit hash of 1881ee28edf4cd81a423ef7f2ca9252a48acb709
 */

static const std::vector<std::pair<std::string, std::string>>
    kLegalStringTests = {{"\"foo\"", "foo"},
                         {"\"foo \\\"bar\\\"\"", "foo \"bar\""}};

static const std::vector<std::pair<std::string, std::string>>
    kLegalBinContentTests = {{"*aGVsbG8=*", "NBSWY3DP"}, {"**", ""}};

static const std::vector<std::pair<std::string, int64_t>> kLegalIntTests = {
    {"42", 42},
    {"0", 0},
    {"00", 0},
    {"-0", 0},
    {"-42", -42},
    {"042", 42},
    {"-042", -42},
    {"9223372036854775807", std::numeric_limits<int64_t>::max()},
    {"-9223372036854775808", std::numeric_limits<int64_t>::min()}};

static const std::vector<std::pair<std::string, double>> kLegalFloatTests = {
    {"1.23", 1.23}, {"-1.23", -1.23}};

static const std::vector<std::string> kIllegalItemTests = {
    "'foo'",
    "\"foo",
    "\"foo \\,\"",
    "\"foo \\",
    "*aGVsbG8*",
    "*aGVsbG8=",
    "*aGVsb G8=*",
    "*aGVsbG!8=*",
    "*aGVsbG!8=!*",
    "*iZ==*",
    "a23",
    "2,3",
    "-a23",
    "4-2",
    "9223372036854775808",
    "-9223372036854775809",
    "1.5.4",
    "1..4"};

static const std::vector<std::string> kIllegalListTests = {"1, 42,", "1,,42"};

class StructuredHeadersStandardTest : public testing::Test {
 public:
  bool decode32(std::string input, std::string& output) {

    uint32_t numBlocks = input.length() / 8;
    uint32_t blockRemainder = input.length() % 8;

    std::string outputBuffer(5, '\0');

    // decode each whole block of the input
    for (uint32_t j = 0; j < numBlocks; j++) {
      if (!decode32Block(input, j, outputBuffer)) {
        return false;
      }
      output += outputBuffer;
    }

    std::string padding(8, '\0');
    // set the initial bytes of the padding to be the trailing bytes of the
    // input string that have been left over
    for (uint32_t i = 0; i < blockRemainder; i++) {
      padding[i] = input[input.size() - blockRemainder + i];
    }

    if (!decode32Block(padding, 0, outputBuffer)) {
      return false;
    }

    // The second argument to the function is the size of the decoded content,
    // where the encoded content is of size (blockRemainder * 5) / 8
    outputBuffer = outputBuffer.substr(0, (blockRemainder * 5) / 8);
    output += outputBuffer;

    return true;
  }

 private:
  /*
   * Applies the following transformation to the input to produce output:
   * for each character in the input, convert it to the byte value of that
   * character as per the base32 alphabet outlined in rfc4648
   */
  std::string convertBase32ToBinary(const std::string& input) {
    std::string base32CharSet("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");
    std::string output;

    for (char c : input) {
      auto it = base32CharSet.find(c);
      if (it == std::string::npos) {
        return "";
      } else {
        // foundCharacter is the position within the base32CharSet where c
        // was found
        char foundCharacter = char(it);
        output.push_back(foundCharacter);
      }
    }

    return output;
  }

  /*
   * Given an input string with length at least 8, and a block number
   * specifying which specific block of the string we are decoding,
   * it sets the outputBuffer to contain the decoded content (which is 5
   * bytes in length). The function assumes that outputBuffer is a string of
   * length 5.
   */
  bool decode32Block(std::string input,
                     uint32_t blockNum,
                     std::string& outputBuffer) {
    CHECK_GE(input.size(), (blockNum + 1) * 8);
    // Remove any padding and make each character of the input represent the
    // byte value of that character, as per the rfc4648 encoding
    boost::trim_right_if(input, [](char c) { return c == '='; });
    input = convertBase32ToBinary(input);
    if (input.empty()) {
      return false;
    }

    // 8 byte buffer
    int64_t buffer = 0;

    for (int i = 0; i < 8; i++) {

      if (input[i + blockNum * 8] >= 32) {
        // a base32 value cannot be greater than or equal to 32
        return false;
      }

      buffer = (buffer << 5);
      buffer = buffer | input[i + blockNum * 8];
    }

    // set the contents of outputBuffer to contain the contents of buffer
    for (int i = 0; i < 5; i++) {
      outputBuffer[i] = char(buffer & 0xFF);
      buffer = buffer >> 8;
    }
    // we perform a reversal because outputBuffer has the contents of buffer
    // but in reverse order
    std::reverse(outputBuffer.begin(), outputBuffer.end());

    return true;
  }
};

class LegalStringTests
    : public StructuredHeadersStandardTest
    , public ::testing::WithParamInterface<
          std::pair<std::string, std::string>> {};

class LegalBinaryContentTests
    : public StructuredHeadersStandardTest
    , public ::testing::WithParamInterface<
          std::pair<std::string, std::string>> {};

class LegalIntegerTests
    : public StructuredHeadersStandardTest
    , public ::testing::WithParamInterface<std::pair<std::string, int64_t>> {};

class LegalFloatTests
    : public StructuredHeadersStandardTest
    , public ::testing::WithParamInterface<std::pair<std::string, double>> {};

class IllegalItemTest
    : public StructuredHeadersStandardTest
    , public ::testing::WithParamInterface<std::string> {};

class IllegalListTest
    : public StructuredHeadersStandardTest
    , public ::testing::WithParamInterface<std::string> {};

TEST_P(LegalStringTests, LegalStrings) {
  std::string input(GetParam().first);
  StructuredHeadersDecoder shd(input);
  StructuredHeaderItem output;
  auto err = shd.decodeItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(output, GetParam().second);
}

TEST_P(LegalBinaryContentTests, LegalBinaryContent) {
  std::string input(GetParam().first);
  std::string expectedOutputInBase32(GetParam().second);
  std::string expectedOutput;
  decode32(expectedOutputInBase32, expectedOutput);

  StructuredHeadersDecoder shd(input);
  StructuredHeaderItem output;
  auto err = shd.decodeItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::BINARYCONTENT);
  EXPECT_EQ(output, expectedOutput);
}

TEST_P(LegalIntegerTests, LegalIntegers) {
  std::string input(GetParam().first);
  StructuredHeadersDecoder shd(input);
  StructuredHeaderItem output;
  auto err = shd.decodeItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(output, GetParam().second);
}

TEST_P(LegalFloatTests, LegalFloats) {
  std::string input(GetParam().first);
  StructuredHeadersDecoder shd(input);
  StructuredHeaderItem output;
  auto err = shd.decodeItem(output);
  EXPECT_EQ(err, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(output.tag, StructuredHeaderItem::Type::DOUBLE);
  EXPECT_EQ(output, GetParam().second);
}

TEST_P(IllegalItemTest, IllegalItem) {
  StructuredHeadersDecoder shd(GetParam());
  StructuredHeaderItem output;
  auto err = shd.decodeItem(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

TEST_P(IllegalListTest, IllegalList) {
  StructuredHeadersDecoder shd(GetParam());
  std::vector<StructuredHeaderItem> output;
  auto err = shd.decodeList(output);
  EXPECT_NE(err, StructuredHeaders::DecodeError::OK);
}

INSTANTIATE_TEST_SUITE_P(TestLegalStrings,
                         LegalStringTests,
                         ::testing::ValuesIn(kLegalStringTests));

INSTANTIATE_TEST_SUITE_P(TestLegalBinaryContent,
                         LegalBinaryContentTests,
                         ::testing::ValuesIn(kLegalBinContentTests));

INSTANTIATE_TEST_SUITE_P(TestLegalInts,
                         LegalIntegerTests,
                         ::testing::ValuesIn(kLegalIntTests));

INSTANTIATE_TEST_SUITE_P(TestLegalFloats,
                         LegalFloatTests,
                         ::testing::ValuesIn(kLegalFloatTests));

INSTANTIATE_TEST_SUITE_P(TestIllegalItems,
                         IllegalItemTest,
                         ::testing::ValuesIn(kIllegalItemTests));

INSTANTIATE_TEST_SUITE_P(TestIllegalLists,
                         IllegalListTest,
                         ::testing::ValuesIn(kIllegalListTests));

TEST_F(StructuredHeadersStandardTest, TestBasicList) {
  std::string input("1, 42");
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  auto err = shd.decodeList(v);
  EXPECT_EQ(err, DecodeError::OK);

  EXPECT_EQ(v.size(), 2);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(v[1].tag, StructuredHeaderItem::Type::INT64);

  EXPECT_EQ(v[0], int64_t(1));
  EXPECT_EQ(v[1], int64_t(42));
}

TEST_F(StructuredHeadersStandardTest, TestSingleItemList) {
  std::string input("42");
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  auto err = shd.decodeList(v);
  EXPECT_EQ(err, DecodeError::OK);

  EXPECT_EQ(v.size(), 1);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::INT64);

  EXPECT_EQ(v[0], int64_t(42));
}

TEST_F(StructuredHeadersStandardTest, TestNoWhitespaceList) {
  std::string input("1,42");
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  auto err = shd.decodeList(v);
  EXPECT_EQ(err, DecodeError::OK);

  EXPECT_EQ(v.size(), 2);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(v[1].tag, StructuredHeaderItem::Type::INT64);

  EXPECT_EQ(v[0], int64_t(1));
  EXPECT_EQ(v[1], int64_t(42));
}

TEST_F(StructuredHeadersStandardTest, TestExtraWhitespaceList) {
  std::string input("1 , 42");
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  auto err = shd.decodeList(v);
  EXPECT_EQ(err, DecodeError::OK);

  EXPECT_EQ(v.size(), 2);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(v[1].tag, StructuredHeaderItem::Type::INT64);

  EXPECT_EQ(v[0], int64_t(1));
  EXPECT_EQ(v[1], int64_t(42));
}

} // namespace proxygen
