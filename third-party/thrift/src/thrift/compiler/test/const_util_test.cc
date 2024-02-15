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

#include <thrift/compiler/lib/const_util.h>

#include <folly/portability/GTest.h>
#include <thrift/compiler/test/gen-cpp2/const_util_test_types.h>

using apache::thrift::compiler::t_const_value;

namespace {
template <typename... Args>
std::unique_ptr<t_const_value> val(Args&&... args) {
  return std::make_unique<t_const_value>(std::forward<Args>(args)...);
}
template <typename Enum, typename = std::enable_if_t<std::is_enum<Enum>::value>>
std::unique_ptr<t_const_value> val(Enum val) {
  return std::make_unique<t_const_value>(
      static_cast<std::underlying_type_t<Enum>>(val));
}
} // namespace

TEST(ConstUtilTest, HydrateConst) {
  auto outer = val();
  outer->set_map();
  auto dbl = val();
  dbl->set_double(42.0);
  outer->add_map(val("floatField"), std::move(dbl));

  auto list = val();
  list->set_list();
  list->add_list(val(42));
  auto inner = val();
  inner->set_map();
  inner->add_map(val("listField"), std::move(list));
  outer->add_map(val("unionField"), std::move(inner));

  outer->add_map(val("enumField"), val(cpp2::E::B));

  cpp2::Outer s;
  hydrate_const(s, *outer);
  EXPECT_EQ(*s.floatField(), 42.0);
  EXPECT_EQ(s.unionField()->listField_ref()->at(0), 42);
  EXPECT_EQ(s.unionField()->listField_ref()->size(), 1);
  EXPECT_EQ(*s.enumField(), cpp2::E::B);
}

TEST(ConstUtilTest, ConstToValue) {
  auto str = val("foo");
  EXPECT_EQ(const_to_value(*str).as_string(), "foo");

  auto map = val();
  map->set_map();
  map->add_map(val("answer"), val(42));
  auto value = const_to_value(*map);
  EXPECT_EQ(value.as_map().at(const_to_value(*val("answer"))).as_i64(), 42);
}
