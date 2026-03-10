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

// Tests JSON decoder backward compatibility for parsing various input formats.
// The JSON5 decoder should accept multiple representations for the same value
// to maintain compatibility with different serialization outputs.

#include <thrift/lib/cpp2/protocol/Json5Protocol.h>

#include <cmath>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types_custom_protocol.h>

namespace apache::thrift {
namespace {

using facebook::thrift::json5::Enum;
using facebook::thrift::json5::Example;

template <typename T>
T parse(std::string_view json) {
  return Json5ProtocolUtils::fromJson5<T>(json);
}

// ── Bool Format Compatibility ────────────────────────────────────────────
// Bools should accept both true/false bare literals and JSON strings.

TEST(JsonDecoderCompatibilityTest, BoolFormats) {
  EXPECT_TRUE(parse<Example>(R"({"boolValue": true})").boolValue().value());
  EXPECT_TRUE(parse<Example>(R"({"boolValue": "true"})").boolValue().value());
  EXPECT_FALSE(parse<Example>(R"({"boolValue": false})").boolValue().value());
  EXPECT_FALSE(parse<Example>(R"({"boolValue": "false"})").boolValue().value());
}

// ── Map Format Compatibility ────────────────────────────────────────────────
// Map should accept both object form {k: v} and array form [{"key": k, "value":
// v}]

TEST(JsonDecoderCompatibilityTest, MapAcceptsObjectForm) {
  auto json = R"({"i32AsKey": {"1": 2, "3": 4}})";
  auto out = Json5ProtocolUtils::fromJson5<Example>(json);
  EXPECT_EQ(
      out.i32AsKey().value(),
      (std::map<std::int32_t, std::int32_t>({{1, 2}, {3, 4}})));
}

TEST(JsonDecoderCompatibilityTest, MapAcceptsArrayForm) {
  auto json =
      R"({"i32AsKey": [{"key": 1, "value": 2}, {"key": 3, "value": 4}]})";
  auto out = Json5ProtocolUtils::fromJson5<Example>(json);
  EXPECT_EQ(
      out.i32AsKey().value(),
      (std::map<std::int32_t, std::int32_t>({{1, 2}, {3, 4}})));
}

// ── Integer Format Compatibility ────────────────────────────────────────────
// Integers should accept both JSON numbers and JSON strings.

TEST(JsonDecoderCompatibilityTest, IntegerFormats) {
  EXPECT_EQ(parse<Example>(R"({"i64Value": 42})").i64Value().value(), 42);
  EXPECT_EQ(parse<Example>(R"({"i64Value": "42"})").i64Value().value(), 42);
  EXPECT_EQ(parse<Example>(R"({"i64Value": "-123"})").i64Value().value(), -123);
}

// ── Float Format Compatibility ──────────────────────────────────────────────
// Floats should accept both JSON numbers and JSON strings.

TEST(JsonDecoderCompatibilityTest, FloatFormats) {
  for (auto json : {R"({"floatValue": 3.14})", R"({"floatValue": "3.14"})"}) {
    EXPECT_EQ(parse<Example>(json).floatValue().value(), 3.14f) << json;
  }
}

TEST(JsonDecoderCompatibilityTest, DoubleFormats) {
  constexpr double kPi = 3.14159265358979;
  for (auto json :
       {R"({"doubleValue": 3.14159265358979})",
        R"({"doubleValue": "3.14159265358979"})"}) {
    EXPECT_EQ(parse<Example>(json).doubleValue().value(), kPi) << json;
  }
}

TEST(JsonDecoderCompatibilityTest, FloatFromIntegers) {
  for (auto json : {R"({"floatValue": 42})", R"({"floatValue": "42"})"}) {
    EXPECT_EQ(parse<Example>(json).floatValue().value(), 42) << json;
  }
}

// ── Enum Format Compatibility ───────────────────────────────────────────────
// Enum should accept "enum-name (enum-value)", "enum-name", "(enum-value)" and
// raw integer

TEST(JsonDecoderCompatibilityTest, EnumFormats) {
  for (auto json : {
           R"_({"enumValue": "ONE (1)"})_",
           R"_({"enumValue": "ONE \t (1)"})_",
           R"({"enumValue": "ONE"})",
           R"_({"enumValue": "(1)"})_",
           R"({"enumValue": 1})",
           R"({"enumValue": 0x1})",
       }) {
    EXPECT_EQ(parse<Example>(json).enumValue().value(), Enum::ONE);
  }

  for (auto json : {
           R"_({"enumValue": "TWO (2)"})_",
           R"_({"enumValue": "(2)"})_",
           R"({"enumValue": 0x2})",
       }) {
    EXPECT_EQ(parse<Example>(json).enumValue().value(), Enum::TWO);
  }

  for (auto json : {
           R"_({"enumValue": "DEFAULT (0)"})_",
           R"_({"enumValue": "(0)"})_",
           R"({"enumValue": 0x0})",
       }) {
    EXPECT_EQ(parse<Example>(json).enumValue().value(), Enum::DEFAULT);
  }

  for (auto json : {
           R"_({"enumValue": "(-5)"})_",
           R"_({"enumValue": -5})_",
       }) {
    EXPECT_EQ(
        folly::to_underlying(parse<Example>(json).enumValue().value()), -5);
  }
}

// ── Enum as Map Key Compatibility ───────────────────────────────────────────
// Enum as map key should accept "enum-name (enum-value)", "enum-name" and
// "(enum-value)"

TEST(JsonDecoderCompatibilityTest, EnumAsKeyAcceptsNameAndValue) {
  auto json = R"json({"enumAsKey": {"ONE (1)": "TWO (2)"}})json";
  auto out = Json5ProtocolUtils::fromJson5<Example>(json);
  EXPECT_EQ(
      out.enumAsKey().value(),
      (std::map<Enum, Enum>({{Enum::ONE, Enum::TWO}})));
}

TEST(JsonDecoderCompatibilityTest, EnumAsKeyAcceptsNameOnly) {
  auto json = R"({"enumAsKey": {"ONE": "TWO"}})";
  auto out = Json5ProtocolUtils::fromJson5<Example>(json);
  EXPECT_EQ(
      out.enumAsKey().value(),
      (std::map<Enum, Enum>({{Enum::ONE, Enum::TWO}})));
}

TEST(JsonDecoderCompatibilityTest, EnumAsKeyAcceptsValueOnly) {
  auto json = R"json({"enumAsKey": {"(1)": "(2)"}})json";
  auto out = Json5ProtocolUtils::fromJson5<Example>(json);
  EXPECT_EQ(
      out.enumAsKey().value(),
      (std::map<Enum, Enum>({{Enum::ONE, Enum::TWO}})));
}

// ── Special Float Values (Infinity/NaN) Compatibility ───────────────────────
// Float/double should accept both JSON5 number form and JSON string form.

TEST(JsonDecoderCompatibilityTest, PositiveInfinityFormats) {
  for (auto json :
       {R"({"infValue": Infinity})", R"({"infValue": "Infinity"})"}) {
    auto value = parse<Example>(json).infValue().value();
    EXPECT_TRUE(std::isinf(value)) << json;
    EXPECT_GT(value, 0) << json;
  }
}

TEST(JsonDecoderCompatibilityTest, NegativeInfinityFormats) {
  for (auto json :
       {R"({"infValue": -Infinity})", R"({"infValue": "-Infinity"})"}) {
    auto value = parse<Example>(json).infValue().value();
    EXPECT_TRUE(std::isinf(value)) << json;
    EXPECT_LT(value, 0) << json;
  }
}

TEST(JsonDecoderCompatibilityTest, NaNFormats) {
  for (auto json : {R"({"nanValue": NaN})", R"({"nanValue": "NaN"})"}) {
    EXPECT_TRUE(std::isnan(parse<Example>(json).nanValue().value())) << json;
  }
}

// ── Binary Encoding Compatibility ───────────────────────────────────────────
// Binary should accept utf-8, base64, and base64url object forms.

TEST(JsonDecoderCompatibilityTest, BinaryBase64Formats) {
  for (auto json : {
           R"({binaryValue: {base64url: "fl9-IQ=="}})",
           R"({binaryValue: {base64url: "fl9-IQ"}})",
           R"({binaryValue: {base64: "fl9+IQ=="}})",
           R"({binaryValue: {base64: "fl9+IQ"}})",
           R"({binaryValue: "fl9-IQ=="})",
           R"({binaryValue: "fl9-IQ"})",
           R"({binaryValue: "fl9+IQ=="})",
           R"({binaryValue: "fl9+IQ"})",
           R"({binaryValue: {"utf-8": "~_~!"}})",
       }) {
    EXPECT_EQ(parse<Example>(json).binaryValue().value(), "~_~!") << json;
  }
}

// ── Bool as Map Key Compatibility ───────────────────────────────────────────
// Bool keys in maps are encoded as strings "true" or "false"

TEST(JsonDecoderCompatibilityTest, BoolAsKeyAcceptsStringTrue) {
  auto json = R"({"boolAsKey": {"true": 42}})";
  auto out = Json5ProtocolUtils::fromJson5<Example>(json);
  EXPECT_EQ(
      out.boolAsKey().value(), (std::map<bool, std::int32_t>({{true, 42}})));
}

TEST(JsonDecoderCompatibilityTest, BoolAsKeyAcceptsStringFalse) {
  auto json = R"({"boolAsKey": {"false": 42}})";
  auto out = Json5ProtocolUtils::fromJson5<Example>(json);
  EXPECT_EQ(
      out.boolAsKey().value(), (std::map<bool, std::int32_t>({{false, 42}})));
}

TEST(JsonDecoderCompatibilityTest, BoolAsKeyBothValues) {
  auto json = R"({"boolAsKey": {"true": 1, "false": 0}})";
  auto out = Json5ProtocolUtils::fromJson5<Example>(json);
  EXPECT_EQ(
      out.boolAsKey().value(),
      (std::map<bool, std::int32_t>({{true, 1}, {false, 0}})));
}

} // namespace
} // namespace apache::thrift
