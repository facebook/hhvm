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
#include <thrift/lib/cpp2/test/gen-cpp2/NativeTypeTest_types.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Testing.h>

namespace apache::thrift::type::test {

using apache::thrift::test::same_type;

TEST(NativeTypeTest, InferTag) {
  same_type<struct_t<MyStruct>, infer_tag<MyStruct>>;
  same_type<MyStruct, native_type<struct_t<MyStruct>>>;
  same_type<union_t<MyUnion>, infer_tag<MyUnion>>;
  same_type<MyUnion, native_type<union_t<MyUnion>>>;
  same_type<exception_t<MyException>, infer_tag<MyException>>;
  same_type<MyException, native_type<exception_t<MyException>>>;
  same_type<enum_t<MyEnum>, infer_tag<MyEnum>>;
  same_type<MyEnum, native_type<enum_t<MyEnum>>>;
}

} // namespace apache::thrift::type::test
