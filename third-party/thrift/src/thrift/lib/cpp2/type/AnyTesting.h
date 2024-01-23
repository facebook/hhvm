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

#pragma once

#include <thrift/lib/cpp2/type/Any.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/test/gen-cpp2/AnyTest1_types.h>

namespace apache::thrift::type {

const auto kMagicNumber = 13370;
const auto kMagicString = "13370";

using Tags = ::testing::Types<
    bool_t,
    byte_t,
    i16_t,
    i32_t,
    i64_t,
    float_t,
    double_t,
    string_t,
    binary_t,
    list<i32_t>,
    set<i32_t>,
    map<i32_t, float_t>,
    struct_t<test::AnyTestStruct>>;

template <class T>
inline const native_type<T> tagToValue = kMagicNumber;

template <>
inline const bool tagToValue<bool_t> = true;

template <>
inline const std::int8_t tagToValue<byte_t> = 0x75;

template <>
inline const std::string tagToValue<string_t> = kMagicString;

template <>
inline const std::string tagToValue<binary_t> = kMagicString;

template <>
inline const std::vector<std::int32_t> tagToValue<list<i32_t>> = {
    kMagicNumber, 0};

template <>
inline const std::set<std::int32_t> tagToValue<set<i32_t>> = {kMagicNumber, 0};

template <>
inline const std::map<std::int32_t, float> tagToValue<map<i32_t, float_t>> = {
    {0, 0}, {kMagicNumber, 0}};

template <>
inline const test::AnyTestStruct tagToValue<struct_t<test::AnyTestStruct>> =
    [] {
      test::AnyTestStruct ret;
      ret.foo() = kMagicNumber;
      return ret;
    }();

template <
    class TypeParam,
    StandardProtocol protocol = StandardProtocol::Compact>
AnyData toAnyData() {
  return AnyData::toAny<TypeParam, protocol>(tagToValue<TypeParam>);
}
} // namespace apache::thrift::type
