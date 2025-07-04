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

#include <thrift/test/reflection/gen-cpp2/reflection_for_each_field.h>
#include <thrift/test/reflection/gen-cpp2/reflection_visit_by_thrift_field_metadata.h> // @manual

#include <gtest/gtest.h>
#include <folly/Overload.h>

#include <typeindex>

namespace test_cpp2::cpp_reflection {
namespace {
template <class T>
auto idToThriftField(int32_t id) {
  apache::thrift::metadata::ThriftField field;
  apache::thrift::for_each_field(T{}, [&](auto&& meta, auto&&) {
    if (meta.id_ref() == id) {
      field = meta;
    }
  });
  return field;
}

TEST(struct1, modify_field) {
  struct1 s;
  s.field4().emplace().set_us("foo");
  s.field5().emplace().set_us_2("bar");
  auto run = folly::overload(
      [](union1& ref) {
        EXPECT_EQ(ref.get_us(), "foo");
        ref.set_ui(20);
      },
      [](auto&) { EXPECT_TRUE(false) << "type mismatch"; });
  auto meta = idToThriftField<struct1>(16);
  apache::thrift::visit_by_thrift_field_metadata(s, meta, [run](auto&& ref) {
    EXPECT_TRUE(ref.has_value());
    run(*ref);
  });
  EXPECT_EQ(s.field4()->ui(), 20);
  EXPECT_EQ(s.field5()->us_2(), "bar");

  meta.id() = 123456;
  EXPECT_THROW(
      apache::thrift::visit_by_thrift_field_metadata(s, meta, [](auto&&...) {}),
      apache::thrift::InvalidThriftId);
}
} // namespace
} // namespace test_cpp2::cpp_reflection
