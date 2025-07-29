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
#include <vector>

#include <gtest/gtest.h>
#include <folly/Memory.h>
#include <folly/Range.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

#include <thrift/test/gen-cpp2/GetterSetterTest_types.h>
#include <thrift/test/gen-cpp2/GetterSetterTest_types_custom_protocol.h>

using namespace thrift::test::getter_setter::cpp2;

TEST(GetterSetter, BasicOptionalFields) {
  GetterSetterTest obj;
  std::vector<int32_t> vec = {1, 2, 3};
  auto buf = std::make_unique<folly::IOBuf>();

  EXPECT_FALSE(obj.optionalInt().has_value());
  EXPECT_EQ(nullptr, apache::thrift::get_pointer(obj.optionalInt()));
  EXPECT_FALSE(obj.optionalList().has_value());
  EXPECT_EQ(nullptr, apache::thrift::get_pointer(obj.optionalList()));
  EXPECT_FALSE(obj.optionalBuf().has_value());
  EXPECT_EQ(nullptr, apache::thrift::get_pointer(obj.optionalBuf()));

  obj.optionalInt() = 42;
  EXPECT_EQ(42, *apache::thrift::get_pointer(obj.optionalInt()));
  EXPECT_TRUE(obj.optionalInt().has_value());
  obj.optionalList() = vec;
  EXPECT_EQ(vec, *apache::thrift::get_pointer(obj.optionalList()));
  EXPECT_TRUE(obj.optionalList().has_value());
  obj.optionalBuf() = std::move(buf);
  EXPECT_TRUE((*apache::thrift::get_pointer(obj.optionalBuf()))->empty());
  EXPECT_TRUE(obj.optionalBuf().has_value());
}

TEST(GetterSetter, BasicDefaultFields) {
  GetterSetterTest obj;
  std::vector<int32_t> vec = {1, 2, 3};
  folly::StringPiece str("abc123");
  auto buf = std::make_unique<folly::IOBuf>(folly::IOBuf::WRAP_BUFFER, str);

  EXPECT_TRUE(obj.defaultList().value().empty());
  EXPECT_EQ(nullptr, obj.defaultBuf().value());

  obj.defaultInt() = 42;
  EXPECT_EQ(42, obj.defaultInt().value());
  obj.defaultList() = vec;
  EXPECT_EQ(vec, obj.defaultList().value());
  obj.defaultBuf() = std::move(buf);
  EXPECT_EQ(6, obj.defaultBuf().value()->length());
}
