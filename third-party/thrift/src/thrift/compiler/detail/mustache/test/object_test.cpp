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

#include <stdexcept>
#include <folly/portability/GTest.h>

#include <thrift/compiler/detail/mustache/mstch.h>

using namespace apache::thrift::mstch;

class test_mstch_object : public internal::object_t<node> {
 public:
  void register_method(
      const std::string& name, const std::function<node()>& method) {
    internal::object_t<node>::register_method(name, method);
  }

  node method1() { return true; }

  node method2() { return std::string("test"); }
};

TEST(MstchObjectTest, callHasForNonExistingMethodShouldReturnFalse) {
  test_mstch_object object;
  object.register_method(
      "method1", std::bind(&test_mstch_object::method1, &object));
  EXPECT_FALSE(object.has("method2"));
}

TEST(MstchObjectTest, callHasForExistingMethodShouldReturnTrue) {
  test_mstch_object object;
  object.register_method(
      "method1", std::bind(&test_mstch_object::method1, &object));
  EXPECT_TRUE(object.has("method1"));
}

TEST(
    MstchObjectTest, callAtForNonExistingMethodShouldThrowOutOfRangeException) {
  test_mstch_object object;
  object.register_method(
      "method1", std::bind(&test_mstch_object::method1, &object));

  auto action = [&object]() { object.at("method2"); };
  EXPECT_THROW(action(), std::out_of_range);
}

TEST(MstchObjectTest, callAtForExistingMethodShouldReturnCorrectValue) {
  test_mstch_object object;
  object.register_method(
      "method1", std::bind(&test_mstch_object::method1, &object));

  auto variantVal = object.at("method1");
  EXPECT_TRUE(std::get<bool>(variantVal));
}

TEST(MstchObjectTest, registerNonExistingMethodShouldSetIt) {
  test_mstch_object object;
  object.register_method(
      "method2", std::bind(&test_mstch_object::method2, &object));

  EXPECT_TRUE(object.has("method2"));
  EXPECT_FALSE(object.has("method1"));
  auto variantVal = object.at("method2");
  EXPECT_EQ(std::get<std::string>(variantVal), "test");
}

TEST(MstchObjectTest, registerExistingMethodShouldThrowRuntimeException) {
  test_mstch_object object;
  object.register_method(
      "method1", std::bind(&test_mstch_object::method1, &object));

  auto action = [&object]() {
    object.register_method(
        "method1", std::bind(&test_mstch_object::method1, &object));
  };

  EXPECT_THROW(action(), std::runtime_error);

  EXPECT_TRUE(object.has("method1"));
  auto variantVal = object.at("method1");
  EXPECT_TRUE(std::get<bool>(variantVal));
}
