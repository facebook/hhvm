/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

#include <thrift/test/gen-cpp2/GetterSetterTest_types.h>
#include <thrift/test/gen-cpp2/GetterSetterTest_types.tcc>

using namespace thrift::test::getter_setter::cpp2;

TEST(GetterSetter, BasicOptionalFields) {
  GetterSetterTest obj;
  std::vector<int32_t> vec = {1, 2, 3};
  auto buf = std::make_unique<folly::IOBuf>();

  EXPECT_FALSE(obj.optionalInt().has_value());
  EXPECT_EQ(nullptr, obj.get_optionalInt());
  EXPECT_FALSE(obj.optionalList().has_value());
  EXPECT_EQ(nullptr, obj.get_optionalList());
  EXPECT_FALSE(obj.optionalBuf().has_value());
  EXPECT_EQ(nullptr, obj.get_optionalBuf());

  obj.optionalInt() = 42;
  EXPECT_EQ(42, *obj.get_optionalInt());
  EXPECT_TRUE(obj.optionalInt().has_value());
  obj.optionalList() = vec;
  EXPECT_EQ(vec, *obj.get_optionalList());
  EXPECT_TRUE(obj.optionalList().has_value());
  obj.optionalBuf() = std::move(buf);
  EXPECT_TRUE((*obj.get_optionalBuf())->empty());
  EXPECT_TRUE(obj.optionalBuf().has_value());
}

TEST(GetterSetter, BasicDefaultFields) {
  GetterSetterTest obj;
  std::vector<int32_t> vec = {1, 2, 3};
  folly::StringPiece str("abc123");
  auto buf = std::make_unique<folly::IOBuf>(folly::IOBuf::WRAP_BUFFER, str);

  EXPECT_TRUE(obj.get_defaultList().empty());
  EXPECT_EQ(nullptr, obj.get_defaultBuf());

  obj.defaultInt() = 42;
  EXPECT_EQ(42, obj.get_defaultInt());
  obj.defaultList() = vec;
  EXPECT_EQ(vec, obj.get_defaultList());
  obj.defaultBuf() = std::move(buf);
  EXPECT_EQ(6, obj.get_defaultBuf()->length());
}
