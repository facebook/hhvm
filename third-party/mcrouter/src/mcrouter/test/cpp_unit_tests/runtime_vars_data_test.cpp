/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include <gtest/gtest.h>

#include <folly/json/json.h>

#include "mcrouter/Observable.h"
#include "mcrouter/RuntimeVarsData.h"

using namespace facebook::memcache::mcrouter;

using std::string;

TEST(runtime_vars_data, sanity) {
  folly::dynamic jsonObj =
      folly::dynamic::object("key1", "value1")("key2", "value2");
  string json = folly::to<string>(folly::toJson(jsonObj));
  RuntimeVarsData obj(json);
  EXPECT_EQ(obj.getVariableByName("key1"), "value1");
  EXPECT_EQ(obj.getVariableByName("key2"), "value2");

  auto newJson = folly::format(
                     "{{\"key3\": \"value3\","
                     "\"key4\": {}}}",
                     json)
                     .str();
  obj = RuntimeVarsData(newJson);
  EXPECT_EQ(obj.getVariableByName("key3"), "value3");
  EXPECT_EQ(obj.getVariableByName("key4"), jsonObj);
  EXPECT_TRUE(obj.getVariableByName("key5").isNull());
}

TEST(runtime_vars_data, register_callback) {
  Observable<std::shared_ptr<const RuntimeVarsData>> obj;
  folly::dynamic jsonObj =
      folly::dynamic::object("key1", "value1")("key2", "value2");
  string json = folly::to<string>(folly::toJson(jsonObj));
  obj.set(std::make_shared<const RuntimeVarsData>(json));
  int counter = 0;
  {
    auto handle = obj.subscribeAndCall(
        [&counter](
            std::shared_ptr<const RuntimeVarsData>,
            std::shared_ptr<const RuntimeVarsData>) { counter++; });
    EXPECT_EQ(counter, 1);
    jsonObj["key2"] = "value3";
    json = folly::to<string>(folly::toJson(jsonObj));
    obj.set(std::make_shared<const RuntimeVarsData>(json));
  }
  EXPECT_EQ(counter, 2);
  auto newJson = folly::format(
                     "{{\"key3\": \"value3\","
                     "\"key4\": {}}}",
                     json)
                     .str();
  obj.set(std::make_shared<const RuntimeVarsData>(newJson));
  EXPECT_EQ(counter, 2);
}
