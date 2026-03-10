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

#include <thrift/lib/cpp2/protocol/Json5Protocol.h>

#include <limits>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_constants.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types.h>

namespace apache::thrift {
namespace {

using namespace facebook::thrift::json5::json5_test_constants;
using facebook::thrift::json5::Example;

enum class JsonFormat { Json, Json5 };

class Json5EncoderTest : public ::testing::TestWithParam<JsonFormat> {
 protected:
  bool isJson5() const { return GetParam() == JsonFormat::Json5; }

  template <class Tag, class T>
  std::string encode(const T& value) {
    if (isJson5()) {
      return Json5ProtocolUtils::toJson5<Tag>(value);
    }
    return Json5ProtocolUtils::toBasicJson<Tag>(value);
  }

  template <class T>
  std::string encode(const T& value) {
    if (isJson5()) {
      return Json5ProtocolUtils::toJson5(value);
    }
    return Json5ProtocolUtils::toBasicJson(value);
  }
};

INSTANTIATE_TEST_SUITE_P(
    /*InstantiationName=*/,
    /*TestSuiteName=*/Json5EncoderTest,
    /*param_generator=*/::testing::Values(JsonFormat::Json, JsonFormat::Json5),
    /*name_generator=*/[](const ::testing::TestParamInfo<JsonFormat>& info) {
      return info.param == JsonFormat::Json5 ? "Json5" : "Json";
    });

TEST_P(Json5EncoderTest, SerializeBool) {
  EXPECT_EQ(encode<type::bool_t>(true), "true");
  EXPECT_EQ(encode<type::bool_t>(false), "false");
}

TEST_P(Json5EncoderTest, SerializeInt) {
  EXPECT_EQ(encode<type::i32_t>(0), "0");
  EXPECT_EQ(encode<type::i32_t>(-0), "0");
  EXPECT_EQ(encode<type::i32_t>(42), "42");
  EXPECT_EQ(encode<type::i32_t>(-42), "-42");
  EXPECT_EQ(
      encode<type::i32_t>(std::numeric_limits<int32_t>::max()), "2147483647");
  EXPECT_EQ(
      encode<type::i32_t>(std::numeric_limits<int32_t>::min()), "-2147483648");
}

TEST_P(Json5EncoderTest, SerializeFloat) {
  EXPECT_EQ(encode<type::double_t>(0.0), "0.0");
  EXPECT_EQ(encode<type::double_t>(1.5), "1.5");
  EXPECT_EQ(encode<type::double_t>(-1.5), "-1.5");
  EXPECT_EQ(encode<type::double_t>(-0.0), "-0.0");
  EXPECT_EQ(
      encode<type::double_t>(
          0.1000000000000000055511151231257827021181583404541015625),
      "0.1");
  EXPECT_EQ(encode<type::float_t>(-0.0), "-0.0");
  EXPECT_EQ(encode<type::float_t>(0.100000001490116119384765625F), "0.1");
}

TEST_P(Json5EncoderTest, SerializeNanInf) {
  if (!isJson5()) {
    EXPECT_EQ(
        encode<type::double_t>(std::numeric_limits<double>::quiet_NaN()),
        R"("NaN")");
    EXPECT_EQ(
        encode<type::double_t>(-std::numeric_limits<double>::quiet_NaN()),
        R"("-NaN")");
    EXPECT_EQ(
        encode<type::double_t>(std::numeric_limits<double>::infinity()),
        R"("Infinity")");
    EXPECT_EQ(
        encode<type::double_t>(-std::numeric_limits<double>::infinity()),
        R"("-Infinity")");
    EXPECT_EQ(
        encode<type::float_t>(std::numeric_limits<float>::quiet_NaN()),
        R"("NaN")");
    EXPECT_EQ(
        encode<type::float_t>(-std::numeric_limits<float>::quiet_NaN()),
        R"("-NaN")");
    EXPECT_EQ(
        encode<type::float_t>(std::numeric_limits<float>::infinity()),
        R"("Infinity")");
    EXPECT_EQ(
        encode<type::float_t>(-std::numeric_limits<float>::infinity()),
        R"("-Infinity")");
  } else {
    EXPECT_EQ(
        encode<type::double_t>(std::numeric_limits<double>::quiet_NaN()),
        "NaN");
    EXPECT_EQ(
        encode<type::double_t>(-std::numeric_limits<double>::quiet_NaN()),
        "-NaN");
    EXPECT_EQ(
        encode<type::double_t>(std::numeric_limits<double>::infinity()),
        "Infinity");
    EXPECT_EQ(
        encode<type::double_t>(-std::numeric_limits<double>::infinity()),
        "-Infinity");
    EXPECT_EQ(
        encode<type::float_t>(std::numeric_limits<float>::quiet_NaN()), "NaN");
    EXPECT_EQ(
        encode<type::float_t>(-std::numeric_limits<float>::quiet_NaN()),
        "-NaN");
    EXPECT_EQ(
        encode<type::float_t>(std::numeric_limits<float>::infinity()),
        "Infinity");
    EXPECT_EQ(
        encode<type::float_t>(-std::numeric_limits<float>::infinity()),
        "-Infinity");
  }
}

TEST_P(Json5EncoderTest, SerializeString) {
  EXPECT_EQ(encode<type::string_t>(std::string("")), R"("")");
  EXPECT_EQ(
      encode<type::string_t>(std::string("Hello, world!")),
      R"("Hello, world!")");
  EXPECT_EQ(
      encode<type::string_t>(std::string(R"(Say "Hello"!)")),
      R"("Say \"Hello\"!")");
  EXPECT_EQ(
      encode<type::string_t>(std::string(R"""(Hello,
world!)""")),
      "\"Hello,\\nworld!\"");
  EXPECT_EQ(encode<type::string_t>(std::string("👋")), R"("👋")");
}

TEST_P(Json5EncoderTest, StringValueInvalidUtf8) {
  Example data;
  data.stringValue() = "\x80\x81\x82";
  EXPECT_THROW((void)encode(data), std::runtime_error);
}

TEST_P(Json5EncoderTest, PrimitiveStructExample) {
  EXPECT_EQ(
      encode(primitiveExample()),
      !isJson5() ? primitiveJson() : primitiveJson5());
}

TEST_P(Json5EncoderTest, NaNInf) {
  Example data;
  data.infValue() = std::numeric_limits<double>::infinity();
  data.nanValue() = std::numeric_limits<double>::quiet_NaN();
  std::string_view expected;
  if (!isJson5()) {
    expected = R"RAW({
  "infValue": "Infinity",
  "nanValue": "NaN"
})RAW";
  } else {
    expected = R"RAW({
  infValue: Infinity,
  nanValue: NaN,
})RAW";
  }
  EXPECT_EQ(encode(data), expected);
}

TEST_P(Json5EncoderTest, NegativeNaNInf) {
  Example data;
  data.infValue() = -std::numeric_limits<double>::infinity();
  data.nanValue() = -std::numeric_limits<double>::quiet_NaN();
  std::string_view expected;
  if (!isJson5()) {
    expected = R"RAW({
  "infValue": "-Infinity",
  "nanValue": "-NaN"
})RAW";
  } else {
    expected = R"RAW({
  infValue: -Infinity,
  nanValue: -NaN,
})RAW";
  }
  EXPECT_EQ(encode(data), expected);
}

TEST_P(Json5EncoderTest, ListValue) {
  EXPECT_EQ(
      encode(listValueExample()),
      !isJson5() ? listValueJson() : listValueJson5());
}

TEST_P(Json5EncoderTest, MultiElementList) {
  EXPECT_EQ(
      encode(multiElementListExample()),
      !isJson5() ? multiElementListJson() : multiElementListJson5());
}

TEST_P(Json5EncoderTest, SetValue) {
  EXPECT_EQ(
      encode(setValueExample()), !isJson5() ? setValueJson() : setValueJson5());
}

TEST_P(Json5EncoderTest, MultiElementSet) {
  EXPECT_EQ(
      encode(multiElementSetExample()),
      !isJson5() ? multiElementSetJson() : multiElementSetJson5());
}

TEST_P(Json5EncoderTest, BoolAsKey) {
  EXPECT_EQ(
      encode(boolAsKeyExample()),
      !isJson5() ? boolAsKeyJson() : boolAsKeyJson5());
}

TEST_P(Json5EncoderTest, I32AsKey) {
  EXPECT_EQ(
      encode(i32AsKeyExample()), !isJson5() ? i32AsKeyJson() : i32AsKeyJson5());
}

TEST_P(Json5EncoderTest, BinaryBase64urlEncoding) {
  EXPECT_EQ(
      encode(binaryBase64Example()),
      !isJson5() ? binaryBase64Json() : binaryBase64Json5());
}

TEST_P(Json5EncoderTest, BinaryAsKey) {
  EXPECT_EQ(
      encode(binaryAsKeyExample()),
      !isJson5() ? binaryAsKeyJson() : binaryAsKeyJson5());
}

TEST_P(Json5EncoderTest, EnumAsKey) {
  EXPECT_EQ(
      encode(enumAsKeyExample()),
      !isJson5() ? enumAsKeyJson() : enumAsKeyJson5());
}

TEST_P(Json5EncoderTest, ListAsKey) {
  EXPECT_EQ(
      encode(listAsKeyExample()),
      !isJson5() ? listAsKeyJson() : listAsKeyJson5());
}

TEST_P(Json5EncoderTest, SetAsKey) {
  EXPECT_EQ(
      encode(setAsKeyExample()), !isJson5() ? setAsKeyJson() : setAsKeyJson5());
}

TEST_P(Json5EncoderTest, MapAsKey) {
  EXPECT_EQ(
      encode(mapAsKeyExample()), !isJson5() ? mapAsKeyJson() : mapAsKeyJson5());
}

TEST_P(Json5EncoderTest, NestedMapAsKey) {
  EXPECT_EQ(
      encode(nestedMapAsKeyExample()),
      !isJson5() ? nestedMapAsKeyJson() : nestedMapAsKeyJson5());
}

TEST_P(Json5EncoderTest, StructAsKey) {
  EXPECT_EQ(
      encode(structAsKeyExample()),
      !isJson5() ? structAsKeyJson() : structAsKeyJson5());
}

TEST_P(Json5EncoderTest, StructWithOutOfOrderFieldIds) {
  EXPECT_EQ(
      encode(outOfOrderFieldsExample()),
      !isJson5() ? outOfOrderFieldsJson() : outOfOrderFieldsJson5());
}

} // namespace
} // namespace apache::thrift
