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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Encode.h>

namespace apache::thrift::op {
namespace {
using apache::thrift::protocol::TType;
using detail::typeTagToTType;

TEST(EncodeTest, TypeTagToTType) {
  EXPECT_EQ(typeTagToTType<type::bool_t>, TType::T_BOOL);
  EXPECT_EQ(typeTagToTType<type::byte_t>, TType::T_BYTE);
  EXPECT_EQ(typeTagToTType<type::i16_t>, TType::T_I16);
  EXPECT_EQ(typeTagToTType<type::i32_t>, TType::T_I32);
  EXPECT_EQ(typeTagToTType<type::i64_t>, TType::T_I64);
  EXPECT_EQ(typeTagToTType<type::float_t>, TType::T_FLOAT);
  EXPECT_EQ(typeTagToTType<type::double_t>, TType::T_DOUBLE);
  EXPECT_EQ(typeTagToTType<type::string_t>, TType::T_UTF7);
  EXPECT_EQ(typeTagToTType<type::binary_t>, TType::T_STRING);
  EXPECT_EQ(typeTagToTType<type::enum_t<void>>, TType::T_I32);
  EXPECT_EQ(typeTagToTType<type::struct_t<void>>, TType::T_STRUCT);
  EXPECT_EQ(typeTagToTType<type::exception_t<void>>, TType::T_STRUCT);
  EXPECT_EQ(typeTagToTType<type::union_t<void>>, TType::T_STRUCT);
  EXPECT_EQ(typeTagToTType<type::list<type::string_t>>, TType::T_LIST);
  EXPECT_EQ(
      (typeTagToTType<type::set<type::list<type::bool_t>>>), TType::T_SET);
  EXPECT_EQ(
      (typeTagToTType<type::map<type::i32_t, type::list<type::bool_t>>>),
      TType::T_MAP);
  // test adapted
  EXPECT_EQ(
      (typeTagToTType<type::adapted<void, type::float_t>>), TType::T_FLOAT);
  EXPECT_EQ(
      (typeTagToTType<type::adapted<void, type::list<type::string_t>>>),
      TType::T_LIST);
  EXPECT_EQ(
      (typeTagToTType<type::adapted<void, type::struct_t<void>>>),
      TType::T_STRUCT);
}
} // namespace
} // namespace apache::thrift::op
