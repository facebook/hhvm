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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_constants.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/json5_test_types_custom_protocol.h>

namespace apache::thrift {
namespace {

using namespace facebook::thrift::json5::json5_test_constants;
using facebook::thrift::json5::Example;

struct TestCase {
  std::string json;
  Example expected;
  std::string name;
};

class Json5DecoderTest : public testing::TestWithParam<TestCase> {};

TEST_P(Json5DecoderTest, Decode) {
  auto out = Json5ProtocolUtils::fromJson5<Example>(GetParam().json);
  EXPECT_EQ(out, GetParam().expected);
}

const TestCase kTestCases[] = {
    {listValueJson(), listValueExample(), "listValueJson"},
    {listValueJson5(), listValueExample(), "listValueJson5"},
    {multiElementListJson(), multiElementListExample(), "multiElementListJson"},
    {multiElementListJson5(),
     multiElementListExample(),
     "multiElementListJson5"},
    {setValueJson(), setValueExample(), "setValueJson"},
    {setValueJson5(), setValueExample(), "setValueJson5"},
    {multiElementSetJson(), multiElementSetExample(), "multiElementSetJson"},
    {multiElementSetJson5(), multiElementSetExample(), "multiElementSetJson5"},
    {i32AsKeyJson(), i32AsKeyExample(), "i32AsKeyJson"},
    {i32AsKeyJson5(), i32AsKeyExample(), "i32AsKeyJson5"},
    {boolAsKeyJson(), boolAsKeyExample(), "boolAsKeyJson"},
    {boolAsKeyJson5(), boolAsKeyExample(), "boolAsKeyJson5"},
    {binaryAsKeyJson(), binaryAsKeyExample(), "binaryAsKeyJson"},
    {binaryAsKeyJson5(), binaryAsKeyExample(), "binaryAsKeyJson5"},
    {enumAsKeyJson(), enumAsKeyExample(), "enumAsKeyJson"},
    {enumAsKeyJson5(), enumAsKeyExample(), "enumAsKeyJson5"},
    {listAsKeyJson(), listAsKeyExample(), "listAsKeyJson"},
    {listAsKeyJson5(), listAsKeyExample(), "listAsKeyJson5"},
    {setAsKeyJson(), setAsKeyExample(), "setAsKeyJson"},
    {setAsKeyJson5(), setAsKeyExample(), "setAsKeyJson5"},
    {mapAsKeyJson(), mapAsKeyExample(), "mapAsKeyJson"},
    {mapAsKeyJson5(), mapAsKeyExample(), "mapAsKeyJson5"},
    {nestedMapAsKeyJson(), nestedMapAsKeyExample(), "nestedMapAsKeyJson"},
    {nestedMapAsKeyJson5(), nestedMapAsKeyExample(), "nestedMapAsKeyJson5"},
    {structAsKeyJson(), structAsKeyExample(), "structAsKeyJson"},
    {structAsKeyJson5(), structAsKeyExample(), "structAsKeyJson5"},
    {binaryBase64Json(), binaryBase64Example(), "binaryBase64Json"},
    {binaryBase64Json5(), binaryBase64Example(), "binaryBase64Json5"},
};

INSTANTIATE_TEST_SUITE_P(
    Decode,
    Json5DecoderTest,
    testing::ValuesIn(kTestCases),
    [](const testing::TestParamInfo<TestCase>& info) {
      return info.param.name;
    });

} // namespace
} // namespace apache::thrift
