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

#include <thrift/lib/cpp2/type/AnyValue.h>

#include <gtest/gtest.h>

#include <folly/Poly.h>

namespace apache::thrift::type {
namespace {

TEST(AnyValueTest, Void) {
  AnyValue value;
  EXPECT_EQ(value.type(), Type::get<void_t>());
  EXPECT_TRUE(value.empty());
  value.clear();
  EXPECT_TRUE(value.empty());

  EXPECT_THROW(value.as<string_t>(), folly::BadPolyCast);
  EXPECT_TRUE(value.try_as<string_t>() == nullptr);
}

TEST(AnyValueTest, Int) {
  AnyValue value = AnyValue::create<i32_t>(1);
  EXPECT_EQ(value.type(), Type::get<i32_t>());
  EXPECT_FALSE(value.empty());
  value.clear();
  EXPECT_TRUE(value.empty());

  EXPECT_EQ(value.as<i32_t>(), 0);
  value.as<i32_t>() = 2;
  EXPECT_FALSE(value.empty());
}

TEST(AnyValueTest, List) {
  AnyValue value;
  value = AnyValue::create<list<string_t>>();
  EXPECT_TRUE(value.empty());
  value.as<list<string_t>>().emplace_back("hi");
  EXPECT_FALSE(value.empty());
  value.clear();
  EXPECT_TRUE(value.empty());
  EXPECT_TRUE(value.as<list<string_t>>().empty());
}

TEST(AnyValueTest, Identical) {
  AnyValue value;
  value = AnyValue::create<float_t>(1.0f);
  EXPECT_FALSE(value.empty());
  value.clear();
  EXPECT_TRUE(value.empty());
  EXPECT_TRUE(value.identical(AnyValue::create<float_t>(0.0f)));
  EXPECT_FALSE(value.identical(AnyValue::create<float_t>(-0.0f)));
  EXPECT_FALSE(value.identical(AnyValue::create<double_t>(0.0)));
}

TEST(AnyRefTest, Initialization) {
  double value = 1.1;
  auto anyValue = AnyValue::create<double_t>(value);
  AnyRef ref1 = value;
  AnyRef ref2 = anyValue;
  AnyConstRef constRef = ref1;
  EXPECT_FALSE(ref1.empty());
  EXPECT_FALSE(ref2.empty());
  EXPECT_TRUE(ref1.type() == ref2.type());
  EXPECT_EQ(ref1.as<double_t>(), value);
  EXPECT_EQ(ref2.as<double_t>(), value);
  EXPECT_EQ(constRef.as<double_t>(), value);
}

TEST(AnyRefTest, Assignment) {
  double value = 1.1;
  AnyRef ref = value;
  ref.as<double_t>() = 1.2;
  EXPECT_EQ(value, 1.2);

  AnyConstRef constRef = value;
  double otherValue = 0.1;
  AnyRef otherRef = otherValue;
  ref = otherRef;
  EXPECT_EQ(ref.as<double_t>(), 0.1);
  constRef = otherValue;
  EXPECT_EQ(constRef.as<double_t>(), 0.1);
  ref = value;
  EXPECT_EQ(ref.as<double_t>(), 1.2);
}

struct IntValue {
  int value;

  bool empty() const { return value == 0; }
  void clear() { value = 0; }
  bool identical(IntValue other) const { return value == other.value; }
  folly::exception_wrapper asExceptionWrapper() const { return {}; }
};

TEST(AnyDataTest, IAnyData) {
  folly::Poly<detail::IAnyData> data = IntValue{1};
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.identical(IntValue{1}));

  data.clear();
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(folly::poly_cast<IntValue&>(data).value, 0);
  EXPECT_TRUE(data.identical(IntValue{0}));
  EXPECT_FALSE(data.identical(IntValue{1}));
}

} // namespace
} // namespace apache::thrift::type
