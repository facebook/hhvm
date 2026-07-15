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

// Drives a read intrinsic over a fixed input string with a small output buffer
// for writer intrinsics.
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
  std::vector<uint8_t> outputBytes() const {
    const auto* end = scratch_.data() + (cursor_.writePos - scratch_.data());
    return std::vector<uint8_t>(scratch_.data(), end);
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

TEST(JsonReadPrimitivesTest, ParseBoolKeyword_ValidTokens) {
  {
    CursorFixture fx(" true ");
    EXPECT_EQ(thrift_transcode_parse_bool_keyword(&fx.cursor()), 1);
    EXPECT_FALSE(fx.errored());
    EXPECT_EQ(fx.consumed(), 5u);
  }
  {
    CursorFixture fx("false]");
    EXPECT_EQ(thrift_transcode_parse_bool_keyword(&fx.cursor()), 0);
    EXPECT_FALSE(fx.errored());
    EXPECT_EQ(fx.consumed(), 5u);
  }
}

TEST(JsonReadPrimitivesTest, ParseBoolKeyword_TrailingGarbageLatchesError) {
  for (const std::string input : {"trueX", "false1"}) {
    CursorFixture fx(input);
    EXPECT_EQ(thrift_transcode_parse_bool_keyword(&fx.cursor()), 0);
    EXPECT_TRUE(fx.errored());
    EXPECT_LE(fx.consumed(), fx.inputSize());
  }
}

TEST(JsonReadPrimitivesTest, ReadJsonStringToken_NoEscapesPointsIntoInput) {
  CursorFixture fx("\"hello\"");
  TranscodeJsonStringToken token{};
  ASSERT_TRUE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  ASSERT_FALSE(fx.errored());
  EXPECT_EQ(
      std::string(
          reinterpret_cast<const char*>(token.begin),
          static_cast<size_t>(token.end - token.begin)),
      "hello");
  EXPECT_EQ(token.hasEscapes, 0);
  EXPECT_TRUE(fx.pointsIntoInput(token.begin));
}

TEST(JsonReadPrimitivesTest, JsonStringTokenEquals_EscapedNewline) {
  CursorFixture fx(R"("a\nb")");
  TranscodeJsonStringToken token{};
  ASSERT_TRUE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  ASSERT_FALSE(fx.errored());
  const std::string expected = "a\nb";
  EXPECT_TRUE(thrift_transcode_json_string_token_equals(
      &token,
      reinterpret_cast<const uint8_t*>(expected.data()),
      expected.size()));
  EXPECT_EQ(token.hasEscapes, 1);
}

TEST(JsonReadPrimitivesTest, WriteJsonStringToken_EscapedNewline) {
  CursorFixture fx(R"("a\nb")");
  TranscodeJsonStringToken token{};
  ASSERT_TRUE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_EQ(thrift_transcode_write_json_string_token(&fx.cursor(), &token), 3u);
  ASSERT_FALSE(fx.errored());
  EXPECT_EQ(fx.output(), "a\nb");
}

TEST(JsonReadPrimitivesTest, WriteJsonStringToken_BmpUnicodeEscape) {
  CursorFixture fx(R"("\u0041")");
  TranscodeJsonStringToken token{};
  ASSERT_TRUE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_EQ(thrift_transcode_write_json_string_token(&fx.cursor(), &token), 1u);
  ASSERT_FALSE(fx.errored());
  EXPECT_EQ(fx.output(), "A");
}

TEST(JsonReadPrimitivesTest, WriteJsonStringToken_SurrogatePairEmoji) {
  CursorFixture fx(R"("\uD83D\uDE00")");
  TranscodeJsonStringToken token{};
  ASSERT_TRUE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_EQ(thrift_transcode_write_json_string_token(&fx.cursor(), &token), 4u);
  ASSERT_FALSE(fx.errored());
  const std::vector<uint8_t> expected = {0xF0, 0x9F, 0x98, 0x80};
  EXPECT_EQ(fx.outputBytes(), expected);
}

TEST(
    JsonReadPrimitivesTest, ReadJsonStringToken_LoneHighSurrogateLatchesError) {
  CursorFixture fx(R"("\uD800")");
  TranscodeJsonStringToken token{};
  EXPECT_FALSE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ReadJsonStringToken_BadHexLatchesError) {
  CursorFixture fx(R"("\uZZ12")");
  TranscodeJsonStringToken token{};
  EXPECT_FALSE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ReadJsonStringToken_UnterminatedLatchesError) {
  CursorFixture fx("\"abc");
  TranscodeJsonStringToken token{};
  EXPECT_FALSE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ReadJsonStringToken_TrailingEscapeLatchesError) {
  CursorFixture fx("\"abc\\");
  TranscodeJsonStringToken token{};
  EXPECT_FALSE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ReadJsonStringToken_UnknownEscapeLatchesError) {
  CursorFixture fx(R"("a\q")");
  TranscodeJsonStringToken token{};
  EXPECT_FALSE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, ReadJsonStringToken_BareControlCharLatchesError) {
  CursorFixture fx("\"a\nb\"");
  TranscodeJsonStringToken token{};
  EXPECT_FALSE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  EXPECT_TRUE(fx.errored());
}

TEST(JsonReadPrimitivesTest, WriteJsonBase64TokenI32Prefixed_DecodesBytes) {
  CursorFixture fx(R"("AQID/w==")");
  TranscodeJsonStringToken token{};
  ASSERT_TRUE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  thrift_transcode_write_json_base64_token_i32_prefixed(&fx.cursor(), &token);
  ASSERT_FALSE(fx.errored());
  EXPECT_EQ(fx.outputBytes(), (std::vector<uint8_t>{0, 0, 0, 4, 1, 2, 3, 255}));
}

TEST(
    JsonReadPrimitivesTest,
    WriteJsonBase64TokenI32Prefixed_DecodesUnpaddedBytes) {
  CursorFixture fx(R"("AQID/w")");
  TranscodeJsonStringToken token{};
  ASSERT_TRUE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  thrift_transcode_write_json_base64_token_i32_prefixed(&fx.cursor(), &token);
  ASSERT_FALSE(fx.errored());
  EXPECT_EQ(fx.outputBytes(), (std::vector<uint8_t>{0, 0, 0, 4, 1, 2, 3, 255}));
}

TEST(
    JsonReadPrimitivesTest,
    WriteJsonBase64TokenI32Prefixed_InvalidInputLatchesError) {
  CursorFixture fx(R"("***")");
  TranscodeJsonStringToken token{};
  ASSERT_TRUE(thrift_transcode_read_json_string_token(&fx.cursor(), &token));
  thrift_transcode_write_json_base64_token_i32_prefixed(&fx.cursor(), &token);
  EXPECT_TRUE(fx.errored());
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

TEST(JsonReadPrimitivesTest, SkipJsonValue_ExcessiveDepthLatchesError) {
  std::string arrays(70, '[');
  arrays.append(70, ']');

  std::string objects;
  objects.reserve(70 * 5 + 4 + 70);
  for (int i = 0; i < 70; ++i) {
    objects.append(R"({"a":)");
  }
  objects.append("null");
  objects.append(70, '}');

  for (const std::string& input : {arrays, objects}) {
    CursorFixture fx(input);
    thrift_transcode_skip_json_value(&fx.cursor());
    EXPECT_TRUE(fx.errored()) << input;
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
