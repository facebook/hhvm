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

// Tests JSON decoder negative cases for rejecting invalid input formats.
// The JSON5 decoder should reject malformed or type-mismatched values
// to ensure strict validation of incoming data.

#include <thrift/lib/cpp2/protocol/Json5Protocol.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types_custom_protocol.h>

namespace apache::thrift {

using facebook::thrift::json5::Example;

struct NegativeTestCase {
  std::string_view name;
  std::string_view json;
};

class JsonDecoderNegativeTest
    : public ::testing::TestWithParam<NegativeTestCase> {};

TEST_P(JsonDecoderNegativeTest, RejectsInvalidInput) {
  EXPECT_THROW(
      (void)Json5ProtocolUtils::fromJson5<Example>(GetParam().json),
      std::exception);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    EnumValidation,
    JsonDecoderNegativeTest,
    ::testing::Values(
        NegativeTestCase{"NameValueMismatch", R"_({"enumValue": "ONE (2)"})_"},
        NegativeTestCase{"NameValueMismatchNegative", R"_({"enumValue": "ONE (-1)"})_"},
        NegativeTestCase{"InvalidIdentifier", R"_({"enumValue": "$ONE"})_"},
        NegativeTestCase{"InvalidName", R"_({"enumValue": "INVALID_ENUM"})_"},
        NegativeTestCase{"InvalidNameWithValue", R"_({"enumValue": "INVALID_ENUM (1)"})_"},
        NegativeTestCase{"ExtraWhitespace", R"_({"enumValue": "ONE "})_"},
        NegativeTestCase{"MissingCloseParen", R"_({"enumValue": "ONE (1"})_"},
        NegativeTestCase{"EmptyParentheses", R"_({"enumValue": "ONE ()"})_"},
        NegativeTestCase{"OnlyOpenParen", R"_({"enumValue": "ONE ("})_"},
        NegativeTestCase{"OnlyCloseParen", R"_({"enumValue": "ONE )"})_"},
        NegativeTestCase{"ParenNotAtEnd", R"_({"enumValue": "ONE (1) extra"})_"},
        NegativeTestCase{"NonIntegerInParens", R"_({"enumValue": "ONE (abc)"})_"},
        NegativeTestCase{"FloatInParens", R"_({"enumValue": "ONE (1.5)"})_"},
        NegativeTestCase{"KeyMismatch", R"_({"enumAsKey": {"ONE (2)": "TWO (2)"}})_"},
        NegativeTestCase{"KeyInvalidName", R"_({"enumAsKey": {"INVALID": "TWO"}})_"},
        NegativeTestCase{"ValueMismatch", R"_({"enumAsKey": {"ONE (1)": "TWO (1)"}})_"},
        NegativeTestCase{"ValueInvalidName", R"_({"enumAsKey": {"ONE (1)": "INVALID (2)"}})_"}),
    [](const auto& info) { return std::string(info.param.name); });

INSTANTIATE_TEST_SUITE_P(
    TypeValidation,
    JsonDecoderNegativeTest,
    ::testing::Values(
        NegativeTestCase{"InvalidBinaryKey", R"_({"binaryValue": {"invalid": "data"}})_"},
        NegativeTestCase{"ListGotString", R"_({"listValue": "not_an_array"})_"},
        NegativeTestCase{"ListGotObject", R"_({"listValue": {"nested": "item1"}})_"},
        NegativeTestCase{"IntegerInvalidBool", R"_({"i64Value": true})_"},
        NegativeTestCase{"IntegerInvalidFloat", R"_({"i64Value": 0.0})_"},
        NegativeTestCase{"IntegerInvalidString", R"_({"i64Value": "123_not_a_number"})_"},
        NegativeTestCase{"IntegerEmptyString", R"_({"i64Value": ""})_"},
        NegativeTestCase{"FloatInvalidBool", R"_({"floatValue": true})_"},
        NegativeTestCase{"FloatInvalidString", R"_({"floatValue": "123_not_a_float"})_"},
        NegativeTestCase{"FloatEmptyString", R"_({"floatValue": ""})_"},
        NegativeTestCase{"FloatPrecisionLoss", R"_({"floatValue": 123456789})_"},
        NegativeTestCase{"FloatPrecisionLossString", R"_({"floatValue": "123456789"})_"},
        NegativeTestCase{"BoolGotNumber", R"_({"boolValue": 1})_"},
        NegativeTestCase{"BoolKeyInvalidString", R"_({"boolAsKey": {"yes": 42}})_"},
        NegativeTestCase{"BoolKeyNumericString", R"_({"boolAsKey": {"1": 42}})_"}),
    [](const auto& info) { return std::string(info.param.name); });

INSTANTIATE_TEST_SUITE_P(
    FormatValidation,
    JsonDecoderNegativeTest,
    ::testing::Values(
        NegativeTestCase{"MapMissingKey", R"_({"i32AsKey": [{"value": 2}]})_"},
        NegativeTestCase{"MapMissingValue", R"_({"i32AsKey": [{"key": 1}]})_"},
        NegativeTestCase{"MapExtraElement", R"_({"i32AsKey": [{"key": 1, "value": 2, "foo": 3}]})_"},
        NegativeTestCase{"MissingClosingBrace", R"_({"i64Value": 42)_"},
        NegativeTestCase{"ExtraComma", R"_({"i64Value": 42,,})_"},
        NegativeTestCase{"EmptyInput", ""},
        NegativeTestCase{"ExtraContent", "{}garbage"}),
    [](const auto& info) { return std::string(info.param.name); });

INSTANTIATE_TEST_SUITE_P(
    OverflowValidation,
    JsonDecoderNegativeTest,
    ::testing::Values(
        // byte overflow (range: -128 to 127)
        NegativeTestCase{"BytePositiveOverflow", R"_({"byteValue": 128})_"},
        NegativeTestCase{"ByteNegativeOverflow", R"_({"byteValue": -129})_"},
        // i16 overflow (range: -32768 to 32767)
        NegativeTestCase{"I16PositiveOverflow", R"_({"i16Value": 32768})_"},
        NegativeTestCase{"I16NegativeOverflow", R"_({"i16Value": -32769})_"},
        // i32 overflow (range: -2147483648 to 2147483647)
        NegativeTestCase{"I32PositiveOverflow", R"_({"i32Value": 2147483648})_"},
        NegativeTestCase{"I32NegativeOverflow", R"_({"i32Value": -2147483649})_"},
        // i64 overflow (range: -9223372036854775808 to 9223372036854775807)
        NegativeTestCase{"I64StringPositiveOverflow", R"_({"i64Value": "9223372036854775808"})_"},
        NegativeTestCase{"I64StringNegativeOverflow", R"_({"i64Value": "-9223372036854775809"})_"}),
    [](const auto& info) { return std::string(info.param.name); });
// clang-format on

} // namespace apache::thrift
