/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transcode/JsonIntrinsics.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include <folly/Conv.h>
#include <folly/portability/GTest.h>

namespace {

// Drives a read intrinsic over a fixed input string, with a scratch write
// buffer large enough that the string unescaper never needs to reallocate.
class CursorFixture {
 public:
  explicit CursorFixture(std::string input) : input_(std::move(input)) {
    const auto* inputBytes = reinterpret_cast<const uint8_t*>(input_.data());
    thrift_transcode_cursor_init(
        &cursor_,
        {inputBytes, inputBytes + input_.size()},
        {scratch_.data(), scratch_.data() + scratch_.size()},
        /*extendFn=*/nullptr,
        /*flushFn=*/nullptr,
        /*userData=*/nullptr);
  }

  TranscodeCursor& cursor() { return cursor_; }
  bool errored() const { return cursor_.error != 0; }
  std::string output() const {
    return std::string(
        reinterpret_cast<const char*>(scratch_.data()),
        static_cast<size_t>(cursor_.writePos - scratch_.data()));
  }

  // Bytes remaining before the current read position, i.e. how far the token
  // scan advanced.
  size_t consumed() const {
    return static_cast<size_t>(
        cursor_.readPos - reinterpret_cast<const uint8_t*>(input_.data()));
  }

  size_t inputSize() const { return input_.size(); }

  bool pointsIntoInput(const uint8_t* p) const {
    const auto* base = reinterpret_cast<const uint8_t*>(input_.data());
    return p >= base && p < base + input_.size();
  }

 private:
  std::string input_;
  std::vector<uint8_t> scratch_ = std::vector<uint8_t>(256);
  TranscodeCursor cursor_{};
};

TEST(JsonReadPrimitivesTest, ParseInt_Positive) {
  CursorFixture fx("123");
  EXPECT_EQ(thrift_transcode_parse_decimal_int(&fx.cursor()), 123);
  EXPECT_FALSE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseInt_Negative) {
  CursorFixture fx("-45");
  EXPECT_EQ(thrift_transcode_parse_decimal_int(&fx.cursor()), -45);
  EXPECT_FALSE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseInt_StopsAtNonDigit) {
  CursorFixture fx("42, next");
  EXPECT_EQ(thrift_transcode_parse_decimal_int(&fx.cursor()), 42);
  EXPECT_FALSE(fx.errored());
  // Scan stopped right after "42", before the comma.
  EXPECT_EQ(fx.consumed(), 2u);
}

TEST(JsonReadPrimitivesTest, ParseInt_FractionalTokenLatchesError) {
  CursorFixture fx("42.0");
  EXPECT_EQ(thrift_transcode_parse_decimal_int(&fx.cursor()), 0);
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseInt_LeadingZeroLatchesError) {
  CursorFixture fx("01");
  EXPECT_EQ(thrift_transcode_parse_decimal_int(&fx.cursor()), 0);
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseInt_OverflowLatchesError) {
  // 21 digits: larger than any int64_t, so the range-checked parse must reject.
  CursorFixture fx("123456789012345678901");
  EXPECT_EQ(thrift_transcode_parse_decimal_int(&fx.cursor()), 0);
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseInt_NonNumericLatchesError) {
  CursorFixture fx("x");
  EXPECT_EQ(thrift_transcode_parse_decimal_int(&fx.cursor()), 0);
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseInt_EmptyLatchesError) {
  CursorFixture fx("");
  EXPECT_EQ(thrift_transcode_parse_decimal_int(&fx.cursor()), 0);
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseFloat_Simple) {
  CursorFixture fx("3.14");
  EXPECT_DOUBLE_EQ(thrift_transcode_parse_decimal_float(&fx.cursor()), 3.14);
  EXPECT_FALSE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseFloat_Exponent) {
  CursorFixture fx("-2.5e10");
  EXPECT_DOUBLE_EQ(thrift_transcode_parse_decimal_float(&fx.cursor()), -2.5e10);
  EXPECT_FALSE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseFloat_QuotedNonFinite) {
  {
    CursorFixture fx(R"("NaN")");
    EXPECT_TRUE(std::isnan(thrift_transcode_parse_decimal_float(&fx.cursor())));
    EXPECT_FALSE(fx.errored());
  }
  {
    CursorFixture fx(R"("Infinity")");
    EXPECT_EQ(
        thrift_transcode_parse_decimal_float(&fx.cursor()),
        std::numeric_limits<double>::infinity());
    EXPECT_FALSE(fx.errored());
  }
  {
    CursorFixture fx(R"("-Infinity")");
    EXPECT_EQ(
        thrift_transcode_parse_decimal_float(&fx.cursor()),
        -std::numeric_limits<double>::infinity());
    EXPECT_FALSE(fx.errored());
  }
}

TEST(JsonReadPrimitivesTest, ParseFloat_BareNonFiniteLatchesError) {
  CursorFixture fx("Infinity");
  EXPECT_EQ(thrift_transcode_parse_decimal_float(&fx.cursor()), 0.0);
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseFloat_LongPrecisionNoTruncation) {
  // Over 70 characters: the old fixed 64-byte buffer would have truncated the
  // token mid-number and parsed a different value.
  const std::string digits =
      "0.1234567890123456789012345678901234567890123456789012345678901234567890123456789";
  ASSERT_GT(digits.size(), 70u);
  const double expected = folly::to<double>(digits);
  CursorFixture fx(digits);
  EXPECT_DOUBLE_EQ(
      thrift_transcode_parse_decimal_float(&fx.cursor()), expected);
  EXPECT_FALSE(fx.errored());
  EXPECT_EQ(fx.consumed(), digits.size());
}

TEST(JsonReadPrimitivesTest, ParseFloat_NonNumericLatchesError) {
  CursorFixture fx("x");
  EXPECT_EQ(thrift_transcode_parse_decimal_float(&fx.cursor()), 0.0);
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ParseString_FastPathPointsIntoInput) {
  CursorFixture fx("\"hello\"");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  ASSERT_FALSE(fx.errored());
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(std::string(reinterpret_cast<const char*>(p), len), "hello");
  // No escapes means no scratch: the result aliases the input buffer.
  EXPECT_TRUE(fx.pointsIntoInput(p));
}

TEST(JsonReadPrimitivesTest, ParseString_EscapedNewline) {
  CursorFixture fx(R"("a\nb")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  ASSERT_FALSE(fx.errored());
  ASSERT_NE(p, nullptr);
  const std::string expected = "a\nb";
  EXPECT_EQ(std::string(reinterpret_cast<const char*>(p), len), expected);
}

TEST(JsonReadPrimitivesTest, ParseString_BmpUnicodeEscape) {
  // A is 'A'.
  CursorFixture fx(R"("\u0041")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  ASSERT_FALSE(fx.errored());
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(std::string(reinterpret_cast<const char*>(p), len), "A");
}

TEST(JsonReadPrimitivesTest, ParseString_SurrogatePairEmoji) {
  // U+1F600 (grinning face) is encoded in JSON as the surrogate pair
  // 😀 and in UTF-8 as F0 9F 98 80.
  CursorFixture fx(R"("\uD83D\uDE00")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  ASSERT_FALSE(fx.errored());
  ASSERT_NE(p, nullptr);
  const std::string bytes(reinterpret_cast<const char*>(p), len);
  const std::vector<uint8_t> expected = {0xF0, 0x9F, 0x98, 0x80};
  const std::vector<uint8_t> actual(bytes.begin(), bytes.end());
  EXPECT_EQ(actual, expected);
}

TEST(JsonReadPrimitivesTest, ParseString_LoneHighSurrogateLatchesError) {
  CursorFixture fx(R"("\uD800")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  EXPECT_TRUE(fx.errored());
  EXPECT_EQ(p, nullptr);
}

TEST(JsonReadPrimitivesTest, ParseString_BadHexLatchesError) {
  CursorFixture fx(R"("\uZZ12")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  EXPECT_TRUE(fx.errored());
  EXPECT_EQ(p, nullptr);
}

TEST(JsonReadPrimitivesTest, ParseString_UnterminatedLatchesError) {
  CursorFixture fx("\"abc");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  EXPECT_TRUE(fx.errored());
  EXPECT_EQ(p, nullptr);
}

TEST(JsonReadPrimitivesTest, ParseString_TrailingEscapeLatchesError) {
  CursorFixture fx("\"abc\\");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  EXPECT_TRUE(fx.errored());
  EXPECT_EQ(p, nullptr);
  EXPECT_EQ(len, 0u);
}

TEST(JsonReadPrimitivesTest, ParseString_UnknownEscapeLatchesError) {
  CursorFixture fx(R"("a\q")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  EXPECT_TRUE(fx.errored());
  EXPECT_EQ(p, nullptr);
  EXPECT_EQ(len, 0u);
}

TEST(JsonReadPrimitivesTest, ParseString_BareControlCharLatchesError) {
  CursorFixture fx("\"a\nb\"");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_escaped_string(&fx.cursor(), &len);
  EXPECT_TRUE(fx.errored());
  EXPECT_EQ(p, nullptr);
  EXPECT_EQ(len, 0u);
}

TEST(JsonReadPrimitivesTest, ParseBase64String_DecodesBytes) {
  CursorFixture fx(R"("AQID/w==")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_base64_string(&fx.cursor(), &len);
  ASSERT_FALSE(fx.errored());
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(
      std::vector<uint8_t>(p, p + len), (std::vector<uint8_t>{1, 2, 3, 255}));
}

TEST(JsonReadPrimitivesTest, ParseBase64String_DecodesUnpaddedBytes) {
  CursorFixture fx(R"("AQID/w")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_base64_string(&fx.cursor(), &len);
  ASSERT_FALSE(fx.errored());
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(
      std::vector<uint8_t>(p, p + len), (std::vector<uint8_t>{1, 2, 3, 255}));
}

TEST(JsonReadPrimitivesTest, ParseBase64String_InvalidInputLatchesError) {
  CursorFixture fx(R"("***")");
  size_t len = 0;
  const uint8_t* p = thrift_transcode_parse_base64_string(&fx.cursor(), &len);
  EXPECT_TRUE(fx.errored());
  EXPECT_EQ(p, nullptr);
  EXPECT_EQ(len, 0u);
}

TEST(JsonReadPrimitivesTest, FormatDecimalInt_Int64Max) {
  CursorFixture fx("");
  thrift_transcode_format_decimal_int(
      &fx.cursor(), std::numeric_limits<int64_t>::max());
  ASSERT_FALSE(fx.errored());
  EXPECT_EQ(fx.output(), "9223372036854775807");
}

TEST(JsonReadPrimitivesTest, SkipJsonValue_ValidNestedValues) {
  for (const std::string input :
       {"true",
        "false",
        "null",
        R"("a\nb")",
        R"({"a":[true,null,-1.5e+2],"b":{"c":"d"}})",
        R"([{"x":"y"},0])"}) {
    CursorFixture fx(input);
    thrift_transcode_skip_json_value(&fx.cursor());
    EXPECT_FALSE(fx.errored()) << input;
    EXPECT_EQ(fx.consumed(), fx.inputSize()) << input;
  }
}

TEST(JsonReadPrimitivesTest, SkipJsonValue_InvalidFormsLatchError) {
  for (const std::string input :
       {R"({1:2})",
        R"({"a" 1})",
        "t",
        "truX",
        "-",
        "1.2.3",
        R"("abc\")",
        R"("a\q")"}) {
    CursorFixture fx(input);
    thrift_transcode_skip_json_value(&fx.cursor());
    EXPECT_TRUE(fx.errored()) << input;
    EXPECT_LE(fx.consumed(), fx.inputSize()) << input;
  }
}

TEST(JsonReadPrimitivesTest, FormatBase64String_EncodesBytes) {
  CursorFixture fx("");
  const std::vector<uint8_t> data = {1, 2, 3, 255};
  thrift_transcode_format_base64_string(&fx.cursor(), data.data(), data.size());
  ASSERT_FALSE(fx.errored());
  EXPECT_EQ(fx.output(), R"("AQID/w==")");
}

} // namespace
