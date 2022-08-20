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

#include <thrift/lib/cpp2/type/TypeRegistry.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Testing.h>
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/lib/cpp2/type/AnyValue.h>
#include <thrift/lib/cpp2/type/Runtime.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Testing.h>
#include <thrift/lib/cpp2/type/UniversalName.h>

namespace apache::thrift::type {
namespace {

TEST(TypeRegistry, Void) {
  TypeRegistry treg;
  // We can store void.
  AnyData data = treg.store<StandardProtocol::Compact>(Ref{});
  EXPECT_EQ(data.type(), Type::get<void_t>());
  EXPECT_TRUE(data.protocol().empty()); // void has no protocol

  // Can load void.
  AnyValue val = treg.load(data);
  EXPECT_TRUE(val.empty());
  EXPECT_EQ(val.type(), Type::get<void_t>());

  // Happy with an empty AnyRef.
  Ref rval;
  treg.load(data, rval);
  EXPECT_TRUE(rval.empty());
  EXPECT_EQ(rval.type(), Type::create<void_t>());

  // Cannot load void into a real reference.
  int32_t uval;
  rval = Ref::to<i32_t>(uval);
  EXPECT_THROW(treg.load(data, rval), std::bad_any_cast);
}

TEST(TypeRegistry, OutOfRange) {
  TypeRegistry treg;
  // Try to store a non-void value.
  EXPECT_THROW(
      (treg.store<i16_t, StandardProtocol::Compact>(7)), std::out_of_range);

  // Try to load a non-void value.
  SemiAny builder;
  builder.type() = i16_t{};
  builder.protocol() = Protocol::get<StandardProtocol::Compact>();
  AnyData data(builder);
  EXPECT_THROW(treg.load(data), std::out_of_range);
}

TEST(TypeRegistry, Register) {
  TypeRegistry treg;
  // Register the protocol
  test::FollyToStringSerializer<double_t> serializer;
  treg.registerSerializer(serializer, double_t{});

  // Store the value using registered protocol.
  AnyData any = treg.store<double_t>(2.5, test::kFollyToStringProtocol);
  EXPECT_EQ(any.type(), Type::get<double_t>());
  EXPECT_EQ(any.protocol().name(), "facebook.com/thrift/FollyToString");
  EXPECT_EQ(any.data().to<std::string>(), "2.5");

  // Load the value, and verify it roundtriped correctly.
  EXPECT_EQ(treg.load(any).as<double_t>(), 2.5);
}

} // namespace
} // namespace apache::thrift::type
