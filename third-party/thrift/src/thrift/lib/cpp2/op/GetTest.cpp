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

#include <thrift/lib/cpp2/op/Get.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/thrift/gen-cpp2/standard_types.h>

namespace apache::thrift::op {
namespace {

TEST(GetTest, GetField) {
  type::UriStruct actual;
  using Tag = type::struct_t<type::UriStruct>;
  actual.scheme() = "foo";
  EXPECT_EQ(*(op::get<>(type::ordinal<1>{}, actual)), "foo");
  EXPECT_EQ(*(op::get<ident::scheme>(actual)), "foo");
  EXPECT_EQ(*(op::get<type::field_id<1>, type::UriStruct>(actual)), "foo");
  EXPECT_EQ(*(op::get<type::field_id<1>, Tag>(actual)), "foo");
}

// O(N) impl for testing.
template <typename T>
FieldId findIdByName(const std::string& name) {
  return find_by_field_id<T>([&](auto id) {
    return op::get_name_v<decltype(id), T> == name ? id() : FieldId{};
  });
}

TEST(GetTest, FindByOrdinal) {
  EXPECT_EQ(findIdByName<type::UriStruct>("unknown"), FieldId{});
  EXPECT_EQ(findIdByName<type::UriStruct>("scheme"), FieldId{1});
}

} // namespace
} // namespace apache::thrift::op
