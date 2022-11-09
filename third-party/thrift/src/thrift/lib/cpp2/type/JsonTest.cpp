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

#include <thrift/lib/cpp2/type/Json.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/type/Testing.h>

namespace apache::thrift::type {

namespace {

TEST(JsonTest, JsonType) {
  EXPECT_EQ(
      folly::to_underlying(Json::Null),
      folly::to_underlying(detail::JsonValue::__EMPTY__));
  EXPECT_EQ(
      folly::to_underlying(Json::Boolean),
      folly::to_underlying(detail::JsonValue::boolValue));
  EXPECT_EQ(
      folly::to_underlying(Json::Number),
      folly::to_underlying(detail::JsonValue::intValue));
  EXPECT_EQ(3, folly::to_underlying(detail::JsonValue::floatValue));
  EXPECT_EQ(
      folly::to_underlying(Json::String),
      folly::to_underlying(detail::JsonValue::stringValue));
  EXPECT_EQ(
      folly::to_underlying(Json::Array),
      folly::to_underlying(detail::JsonValue::arrayValue));
  EXPECT_EQ(
      folly::to_underlying(Json::Object),
      folly::to_underlying(detail::JsonValue::objectValue));
}

TEST(JsonTest, Null) {
  Json json;
  EXPECT_EQ(json.type(), Json::Null);
}

TEST(JsonTest, Boolean) {
  Json json;
  json = false;
  EXPECT_EQ(json.type(), Json::Boolean);
  EXPECT_FALSE(*json.boolean());
  json = true;
  EXPECT_EQ(json.type(), Json::Boolean);
  EXPECT_TRUE(*json.boolean());
}

TEST(JsonTest, String) {
  Json json;
  json = "";
  EXPECT_EQ(json.type(), Json::String);
  EXPECT_EQ(*json.string(), "");
  json = "hi";
  EXPECT_EQ(json.type(), Json::String);
  EXPECT_EQ(*json.string(), "hi");
}

TEST(JsonTest, Array) {
  Json json;
  json[0] = "hi";
  json[Ordinal(3)] = 5.5;
  JsonArray& array = *json.array();
  EXPECT_EQ(array.size(), 3);
  EXPECT_EQ(array[0].type(), Json::String);
  EXPECT_EQ(array[1].type(), Json::Null);
  EXPECT_EQ(array[2].type(), Json::Number);
}

TEST(JsonTest, Object) {
  Json json;
  json["hi"] = 1.0;
  EXPECT_EQ(json["hi"].type(), Json::Number);
  EXPECT_FALSE(json.isNumber());
  EXPECT_TRUE(json["hi"].isNumber());
  EXPECT_TRUE(json["hi"].isInt());
  EXPECT_FALSE(json["hi"].isFloat());
  EXPECT_EQ(*json["hi"].integer(), 1);
  EXPECT_FALSE(json["hi"].floating().has_value());

  json = 1.1;
  EXPECT_TRUE(json.isNumber());
  EXPECT_FALSE(json.isInt());
  EXPECT_TRUE(json.isFloat());

  EXPECT_THROW(json["bye"], thrift::bad_field_access);
  JsonObject& obj = json.ensure<Json::Object>();
  EXPECT_EQ(json.type(), Json::Object);
  obj["bye"] = true;
  EXPECT_EQ(json.at("bye").type(), Json::Boolean);
}

} // namespace
} // namespace apache::thrift::type
