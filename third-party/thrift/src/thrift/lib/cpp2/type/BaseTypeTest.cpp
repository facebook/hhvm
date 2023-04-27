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

#include <thrift/lib/cpp2/type/BaseType.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>

namespace apache::thrift::type {
namespace {
using protocol::TType;

TEST(TraitsTest, Conversions) {
  EXPECT_EQ(toTType(BaseType::Bool), TType::T_BOOL);
  EXPECT_EQ(toBaseType(TType::T_BOOL), BaseType::Bool);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Bool), BaseType::Bool);

  EXPECT_EQ(toTType(BaseType::Byte), TType::T_BYTE);
  EXPECT_EQ(toBaseType(TType::T_BYTE), BaseType::Byte);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Byte), BaseType::Byte);

  EXPECT_EQ(toTType(BaseType::I16), TType::T_I16);
  EXPECT_EQ(toBaseType(TType::T_I16), BaseType::I16);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::I16), BaseType::I16);

  EXPECT_EQ(toTType(BaseType::I32), TType::T_I32);
  EXPECT_EQ(toBaseType(TType::T_I32), BaseType::I32);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::I32), BaseType::I32);

  EXPECT_EQ(toTType(BaseType::I64), TType::T_I64);
  EXPECT_EQ(toBaseType(TType::T_I64), BaseType::I64);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::I64), BaseType::I64);

  EXPECT_EQ(toTType(BaseType::Enum), TType::T_I32);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Enum), BaseType::Enum);

  EXPECT_EQ(toTType(BaseType::Float), TType::T_FLOAT);
  EXPECT_EQ(toBaseType(TType::T_FLOAT), BaseType::Float);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Float), BaseType::Float);

  EXPECT_EQ(toTType(BaseType::Double), TType::T_DOUBLE);
  EXPECT_EQ(toBaseType(TType::T_DOUBLE), BaseType::Double);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Double), BaseType::Double);

  EXPECT_EQ(toTType(BaseType::String), TType::T_UTF8);
  EXPECT_EQ(toBaseType(TType::T_UTF8), BaseType::String);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::String), BaseType::String);

  EXPECT_EQ(toTType(BaseType::Binary), TType::T_STRING);
  EXPECT_EQ(toBaseType(TType::T_STRING), BaseType::Binary);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Binary), BaseType::Binary);

  EXPECT_EQ(toTType(BaseType::Struct), TType::T_STRUCT);
  EXPECT_EQ(toBaseType(TType::T_STRUCT), BaseType::Struct);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Struct), BaseType::Struct);

  EXPECT_EQ(toTType(BaseType::Union), TType::T_STRUCT);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Union), BaseType::Union);

  EXPECT_EQ(toTType(BaseType::Exception), TType::T_STRUCT);
  EXPECT_EQ(
      static_cast<BaseType>(BaseTypeEnum::Exception), BaseType::Exception);

  EXPECT_EQ(toTType(BaseType::List), TType::T_LIST);
  EXPECT_EQ(toBaseType(TType::T_LIST), BaseType::List);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::List), BaseType::List);

  EXPECT_EQ(toTType(BaseType::Set), TType::T_SET);
  EXPECT_EQ(toBaseType(TType::T_SET), BaseType::Set);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Set), BaseType::Set);

  EXPECT_EQ(toTType(BaseType::Map), TType::T_MAP);
  EXPECT_EQ(toBaseType(TType::T_MAP), BaseType::Map);
  EXPECT_EQ(static_cast<BaseType>(BaseTypeEnum::Map), BaseType::Map);
}

} // namespace
} // namespace apache::thrift::type
