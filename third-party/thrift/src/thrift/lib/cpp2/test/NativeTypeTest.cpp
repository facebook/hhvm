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

#include <type_traits>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/test/gen-cpp2/NativeTypeTest_types.h>
#include <thrift/lib/cpp2/type/NativeType.h>

namespace apache::thrift::type::test {

TEST(NativeTypeTest, InferTag) {
  static_assert(std::is_same_v<struct_t<MyStruct>, infer_tag<MyStruct>>);
  static_assert(std::is_same_v<MyStruct, native_type<struct_t<MyStruct>>>);
  static_assert(std::is_same_v<union_t<MyUnion>, infer_tag<MyUnion>>);
  static_assert(std::is_same_v<MyUnion, native_type<union_t<MyUnion>>>);
  static_assert(
      std::is_same_v<exception_t<MyException>, infer_tag<MyException>>);
  static_assert(
      std::is_same_v<MyException, native_type<exception_t<MyException>>>);
  static_assert(std::is_same_v<enum_t<MyEnum>, infer_tag<MyEnum>>);
  static_assert(std::is_same_v<MyEnum, native_type<enum_t<MyEnum>>>);
}

} // namespace apache::thrift::type::test
