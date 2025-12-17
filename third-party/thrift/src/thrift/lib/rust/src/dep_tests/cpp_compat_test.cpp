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

#include <memory>
#include <common/gtest/gtest_extensions.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/rust/src/dep_tests/gen-cpp2/test_thrift_constants.h>
#include <thrift/lib/rust/src/dep_tests/gen-cpp2/test_thrift_types.h>

namespace facebook {

TEST(JsonTest, structKey) {
  Small s;
  SubStruct stru;
  std::map<Small, int32_t> m;
  m.insert({s, 1});
  stru.key_map() = m;

  SubStruct outStruct;

  auto t = apache::thrift::SimpleJSONSerializer::serialize<std::string>(stru);
  ASSERT_EQ(
      t,
      "{\"req_def\":\"IAMREQ\",\"key_map\":{{\"num\":0,\"two\":0}:1},\"bin\":\"\"}");

  apache::thrift::SimpleJSONSerializer::deserialize(t, outStruct);
  ASSERT_EQ(stru, outStruct);
}

TEST(JsonTest, weirdText) {
  SubStruct stru;
  stru.optDef() = "stuff\twith\nescape\\characters'...\"lots{of}fun</xml>";
  stru.bin() = "1234";

  SubStruct outStruct;

  auto t = apache::thrift::SimpleJSONSerializer::serialize<std::string>(stru);
  ASSERT_EQ(
      t,
      "{\"optDef\":\"stuff\\twith\\nescape\\\\characters'...\\\"lots{of}fun</xml>\","
      "\"req_def\":\"IAMREQ\",\"bin\":\"MTIzNA\"}");

  apache::thrift::SimpleJSONSerializer::deserialize(t, outStruct);
  ASSERT_EQ(stru, outStruct);

  stru.optDef() = "UNICODE\U0001F60AUH OH";
  t = apache::thrift::SimpleJSONSerializer::serialize<std::string>(stru);
  ASSERT_EQ(
      t,
      "{\"optDef\":\"UNICODE\xf0\x9f\x98\x8aUH OH\","
      "\"req_def\":\"IAMREQ\",\"bin\":\"MTIzNA\"}");

  apache::thrift::SimpleJSONSerializer::deserialize(t, outStruct);
  ASSERT_EQ(stru, outStruct);
}

TEST(JsonTest, skipComplex) {
  SubStruct stru;
  stru.optDef() = "thing";
  stru.bin() = "1234";

  SubStruct outStruct;

  std::string input(
      "{\"optDef\":\"thing\","
      "\"req_def\":\"IAMREQ\",\"bin\":\"MTIzNA\","
      "\"extra\":[1, {\"thing\":\"thing2\"}],"
      "\"extra_map\":{\"thing\":null,\"thing2\":2},"
      "\"extra_bool\":true}");

  apache::thrift::SimpleJSONSerializer::deserialize(input, outStruct);
  ASSERT_EQ(stru, outStruct);
}

TEST(JsonTest, needCommas) {
  Small outStruct;

  // Note the missing commas

  std::string input(
      "{\"num\":1"
      "\"two\":2}");

  EXPECT_THROW(
      apache::thrift::SimpleJSONSerializer::deserialize(input, outStruct),
      std::exception);

  // even when skipping
  std::string input2(
      "{\"num\":1,"
      "\"two\":2,"
      "\"extra_map\":{\"thing\":null,\"thing2\":2}"
      "\"extra_bool\":true}");

  EXPECT_THROW(
      apache::thrift::SimpleJSONSerializer::deserialize(input2, outStruct),
      std::exception);
}

TEST(JsonTest, needCommasContainers) {
  Containers outStruct;

  std::string goodinput(R"({"m":{"m1":"m1","m2":"m2"}})");
  // Note the missing comma
  std::string badinput(R"({"m":{"m1":"m1""m2":"m2"}})");
  // we can serialize the good input
  apache::thrift::SimpleJSONSerializer::deserialize(goodinput, outStruct);
  EXPECT_THROW(
      apache::thrift::SimpleJSONSerializer::deserialize(badinput, outStruct),
      std::exception);

  std::string goodinput2(R"({"l":["l1","l2"]})");
  // Note the missing comma
  std::string badinput2(R"({"l":["l1""l2"]})");
  // we can serialize the good input
  apache::thrift::SimpleJSONSerializer::deserialize(goodinput2, outStruct);
  EXPECT_THROW(
      apache::thrift::SimpleJSONSerializer::deserialize(badinput2, outStruct),
      std::exception);
}

TEST(JsonTest, nullStuffDeser) {
  // NOTE: Keep in sync with `test_null_stuff_deser` test in simplejson.rs

  SubStruct stru;
  stru.bin() = "1234";

  const std::string_view inputs[] = {
      R"({                "req_def": "IAMREQ", "bin": "MTIzNA" })",
      R"({"optDef": null, "req_def": "IAMREQ", "bin": "MTIzNA" })",
  };

  for (const auto& input : inputs) {
    SubStruct outStruct;
    apache::thrift::SimpleJSONSerializer::deserialize(input, outStruct);
    EXPECT_EQ(stru, outStruct) << input;
  }
}

TEST(JsonTest, unknownUnion) {
  Un u;

  auto t = apache::thrift::SimpleJSONSerializer::serialize<std::string>(u);
  ASSERT_EQ(t, "{}");
}

TEST(DefaultsTest, defaults) {
  // NOTE: Keep in sync with `test_defaults` test in defaults.rs

  SubStruct stru;
  ASSERT_FALSE(stru.optDef().has_value());
  ASSERT_EQ("IAMREQ", stru.req_def());
  ASSERT_FALSE(stru.key_map().has_value());
  ASSERT_TRUE(stru.bin()->empty());
}

TEST(ConstsTest, consts) {
  // NOTE: Keep in sync with `test_consts` test in consts.rs

  const auto def = test_thrift_constants::DEFAULT_SUBSTRUCT();
  ASSERT_EQ(SubStruct(), def);

  const auto custom = test_thrift_constants::CUSTOM_SUBSTRUCT();
  SubStruct expected;
  expected.optDef() = "CUSTOM_OPT_DEF";
  expected.req_def() = "CUSTOM_REQ_DEF";
  Small key;
  key.num() = 4;
  key.two() = 2;
  expected.key_map() = {{key, 42}};
  expected.bin() = "0123456789";
  ASSERT_EQ(expected, custom);
}

} // namespace facebook
