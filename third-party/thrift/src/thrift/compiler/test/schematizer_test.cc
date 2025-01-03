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

#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/sema/schematizer.h>

#include <folly/portability/GTest.h>

using apache::thrift::compiler::t_const_value;
using apache::thrift::compiler::t_enum;
using apache::thrift::compiler::t_enum_value;
using apache::thrift::compiler::t_field;
using apache::thrift::compiler::t_list;
using apache::thrift::compiler::t_map;
using apache::thrift::compiler::t_primitive_type;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_set;
using apache::thrift::compiler::t_struct;
using apache::thrift::compiler::t_type_ref;
using apache::thrift::compiler::t_typedef;
using apache::thrift::compiler::detail::protocol_value_builder;

template <typename... Args>
std::unique_ptr<t_const_value> val(Args&&... args) {
  return std::make_unique<t_const_value>(std::forward<Args>(args)...);
}

std::unique_ptr<t_const_value> val(double d) {
  auto d_val = std::make_unique<t_const_value>();
  d_val->set_double(d);
  return d_val;
}

std::unique_ptr<t_const_value> val(std::string_view s) {
  return val(std::string{s});
}

template <typename Enm, typename = std::enable_if_t<std::is_enum_v<Enm>>>
std::unique_ptr<t_const_value> val(Enm val) {
  return std::make_unique<t_const_value>(
      static_cast<std::underlying_type_t<Enm>>(val));
}

TEST(SchematizerTest, wrap_with_protocol_value) {
  t_const_value str("foo");
  auto value = protocol_value_builder::as_value_type().wrap(str, {});
  auto map = value->get_map();
  EXPECT_EQ(map.at(0).first->get_string(), "stringValue");
  EXPECT_EQ(map.at(0).second->get_string(), "foo");
}

enum class MyEnum : std::uint16_t { Foo = 1, Bar = 2 };

template <typename T>
void expectWrappedValue(
    const t_const_value& wrappedValue,
    const t_const_value::t_const_value_kind kind,
    std::string_view label,
    T literalValue) {
  const auto& [key, val] = wrappedValue.get_map().at(0);
  EXPECT_TRUE(key->kind() == t_const_value::CV_STRING);
  EXPECT_EQ(key->get_string(), label);
  EXPECT_TRUE(val->kind() == kind);
  switch (kind) {
    case t_const_value::CV_INTEGER:
      EXPECT_EQ(literalValue, val->get_integer());
      break;
    case t_const_value::CV_DOUBLE:
      EXPECT_EQ(literalValue, val->get_double());
      break;
    case t_const_value::CV_BOOL:
      EXPECT_EQ(literalValue, val->get_bool());
      break;
    case t_const_value::CV_STRING:
    case t_const_value::CV_MAP:
    case t_const_value::CV_LIST:
    case t_const_value::CV_IDENTIFIER:
      throw std::runtime_error("Unimplemented comparison");
  }
}

TEST(SchematizerTest, wrap_with_protocol_passthrough) {
  auto strct = t_const_value::make_map();
  strct->add_map(val("foo"), val(1));
  strct->add_map(val("bar"), val(MyEnum::Bar));
  strct->add_map(val("baz"), val(3.14));
  strct->add_map(val("qux"), val(4.2));

  // Without type mapping, integers will be mapped to i64.
  auto value_no_type_mapping =
      protocol_value_builder::as_value_type().wrap(*strct, {});
  const auto& value_map =
      value_no_type_mapping->get_map().at(0).second->get_map();
  auto [foo, foo_val] = value_map.at(0);
  auto [bar, bar_val] = value_map.at(1);
  auto [baz, baz_val] = value_map.at(2);
  auto [qux, qux_val] = value_map.at(3);
  expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i64Value", 1);
  expectWrappedValue(*bar_val, t_const_value::CV_INTEGER, "i64Value", 2);
  expectWrappedValue(*baz_val, t_const_value::CV_DOUBLE, "doubleValue", 3.14);
  expectWrappedValue(*qux_val, t_const_value::CV_DOUBLE, "doubleValue", 4.2);
}

std::unique_ptr<t_struct> make_foo_bar(const t_program* program) {
  auto struct_ty = std::make_unique<t_struct>(program, "FooBar");
  struct_ty->append_field(
      std::make_unique<t_field>(&t_primitive_type::t_i32(), "foo", 1));
  struct_ty->append_field(
      std::make_unique<t_field>(&t_primitive_type::t_i16(), "bar", 2));
  struct_ty->append_field(
      std::make_unique<t_field>(&t_primitive_type::t_float(), "baz", 3));
  struct_ty->append_field(
      std::make_unique<t_field>(&t_primitive_type::t_double(), "qux", 4));
  return struct_ty;
}

TEST(SchematizerTest, wrap_with_protocol_with_struct_ty) {
  auto strct = t_const_value::make_map();
  strct->add_map(val("foo"), val(1));
  strct->add_map(val("bar"), val(2));
  strct->add_map(val("baz"), val(3.14));
  strct->add_map(val("qux"), val(4.2));

  // With the type mapping, integers will be represented by their types in
  // FooBar.
  auto program = std::make_unique<t_program>("./", "./");
  auto foo_bar_ty = make_foo_bar(program.get());
  auto value_no_type_mapping =
      protocol_value_builder{*foo_bar_ty.get()}.wrap(*strct, {});
  const auto& value_map =
      value_no_type_mapping->get_map().at(0).second->get_map();
  auto [foo, foo_val] = value_map.at(0);
  auto [bar, bar_val] = value_map.at(1);
  auto [baz, baz_val] = value_map.at(2);
  auto [qux, qux_val] = value_map.at(3);
  expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i32Value", 1);
  expectWrappedValue(*bar_val, t_const_value::CV_INTEGER, "i16Value", 2);
  expectWrappedValue(*baz_val, t_const_value::CV_DOUBLE, "floatValue", 3.14);
  expectWrappedValue(*qux_val, t_const_value::CV_DOUBLE, "doubleValue", 4.2);
}

std::unique_ptr<t_struct> make_foo_enum(
    const t_program* program, const t_enum* my_enum) {
  auto struct_ty = std::make_unique<t_struct>(program, "FooBar");
  struct_ty->append_field(std::make_unique<t_field>(my_enum, "foo", 1));
  return struct_ty;
}

TEST(SchematizerTest, wrap_with_protocol_with_enum_ty) {
  auto strct = t_const_value::make_map();
  strct->add_map(val("foo"), val(MyEnum::Foo));

  // Enums will be represented by an i64 value.
  auto program = std::make_unique<t_program>("./", "./");
  auto my_enum = std::make_unique<t_enum>(program.get(), "MyEnum");
  my_enum->append_value(std::make_unique<t_enum_value>("Foo"));
  my_enum->append_value(std::make_unique<t_enum_value>("Bar"));
  auto foo_enum_ty = make_foo_enum(program.get(), my_enum.get());
  auto value_no_type_mapping =
      protocol_value_builder{*foo_enum_ty.get()}.wrap(*strct, {});
  const auto& value_map =
      value_no_type_mapping->get_map().at(0).second->get_map();
  auto [foo, foo_val] = value_map.at(0);
  expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i32Value", 1);
}

std::unique_ptr<t_list> make_foo_bar_list(const t_struct* element) {
  return std::make_unique<t_list>(element);
}

TEST(SchematizerTest, wrap_with_protocol_list) {
  auto program = std::make_unique<t_program>("./", "./");
  auto foo_bar_strct_ty = make_foo_bar(program.get());
  auto foo_bar_list_ty = make_foo_bar_list(foo_bar_strct_ty.get());
  auto struct_with_list_ty =
      std::make_unique<t_struct>(program.get(), "FooBarList");
  struct_with_list_ty->append_field(
      std::make_unique<t_field>(foo_bar_list_ty.get(), "foos", 1));

  auto my_list = t_const_value::make_list();
  auto foo_bar_elem = t_const_value::make_map();
  foo_bar_elem->add_map(val("foo"), val(1));
  foo_bar_elem->add_map(val("bar"), val(2));
  my_list->add_list(foo_bar_elem.get()->clone());
  my_list->add_list(std::move(foo_bar_elem));

  auto foos = t_const_value::make_map();
  foos->add_map(val("foos"), std::move(my_list));

  auto value_no_type_mapping =
      protocol_value_builder{*struct_with_list_ty}.wrap(*foos, {});
  const auto& value_map =
      value_no_type_mapping->get_map().at(0).second->get_map();
  auto [foos_elem, foos_val] = value_map.at(0);
  EXPECT_EQ(foos_val->get_map().at(0).first->get_string(), "listValue");
  const auto& foos_list = foos_val->get_map().at(0).second->get_list();
  EXPECT_EQ(foos_list.size(), 2);

  {
    const auto& [li_key, li_val] = foos_list.at(0)->get_map().at(0);
    auto [foo, foo_val] = li_val->get_map().at(0);
    auto [bar, bar_val] = li_val->get_map().at(1);
    expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i32Value", 1);
    expectWrappedValue(*bar_val, t_const_value::CV_INTEGER, "i16Value", 2);
  }
  {
    const auto& [li_key, li_val] = foos_list.at(0)->get_map().at(0);
    auto [foo, foo_val] = li_val->get_map().at(0);
    auto [bar, bar_val] = li_val->get_map().at(1);
    expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i32Value", 1);
    expectWrappedValue(*bar_val, t_const_value::CV_INTEGER, "i16Value", 2);
  }
}

TEST(SchematizerTest, wrap_with_protocol_set) {
  auto program = std::make_unique<t_program>("./", "./");
  auto foo_bar_strct_ty = make_foo_bar(program.get());
  auto foo_bar_set_ty = std::make_unique<t_set>(foo_bar_strct_ty.get());
  auto struct_with_set_ty =
      std::make_unique<t_struct>(program.get(), "FooBarSet");
  struct_with_set_ty->append_field(
      std::make_unique<t_field>(foo_bar_set_ty.get(), "foos", 1));

  auto my_set = t_const_value::make_list();
  auto foo_bar_elem_1 = t_const_value::make_map();
  foo_bar_elem_1->add_map(val("foo"), val(1));
  foo_bar_elem_1->add_map(val("bar"), val(2));
  my_set->add_list(std::move(foo_bar_elem_1));
  auto foo_bar_elem_2 = t_const_value::make_map();
  foo_bar_elem_2->add_map(val("foo"), val(4));
  foo_bar_elem_2->add_map(val("bar"), val(5));
  my_set->add_list(std::move(foo_bar_elem_2));

  auto foos = t_const_value::make_map();
  foos->add_map(val("foos"), std::move(my_set));

  auto value_no_type_mapping =
      protocol_value_builder{*struct_with_set_ty}.wrap(*foos, {});
  const auto& value_map =
      value_no_type_mapping->get_map().at(0).second->get_map();
  auto [foos_elem, foos_val] = value_map.at(0);
  EXPECT_EQ(foos_val->get_map().at(0).first->get_string(), "setValue");
  const auto& foos_set = foos_val->get_map().at(0).second->get_list();
  EXPECT_EQ(foos_set.size(), 2);
  {
    const auto& [set_key, set_val] = foos_set.at(0)->get_map().at(0);
    auto [foo, foo_val] = set_val->get_map().at(0);
    auto [bar, bar_val] = set_val->get_map().at(1);
    expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i32Value", 1);
    expectWrappedValue(*bar_val, t_const_value::CV_INTEGER, "i16Value", 2);
  }
  {
    const auto& [set_key, set_val] = foos_set.at(1)->get_map().at(0);
    auto [foo, foo_val] = set_val->get_map().at(0);
    auto [bar, bar_val] = set_val->get_map().at(1);
    expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i32Value", 4);
    expectWrappedValue(*bar_val, t_const_value::CV_INTEGER, "i16Value", 5);
  }
}

TEST(SchematizerTest, wrap_with_protocol_typedef) {
  // Typedefs will be resolved to the underlying type.

  auto program = std::make_unique<t_program>("./", "./");
  auto foo_bar_strct_ty = make_foo_bar(program.get());
  auto foo_bar_typedef = std::make_unique<t_typedef>(
      program.get(), "FooBarAlias", t_type_ref{*foo_bar_strct_ty});

  auto strct = t_const_value::make_map();
  strct->add_map(val("foo"), val(1));
  strct->add_map(val("bar"), val(2));

  auto value_no_type_mapping =
      protocol_value_builder{*foo_bar_typedef}.wrap(*strct, {});
  const auto& value_map =
      value_no_type_mapping->get_map().at(0).second->get_map();
  auto [foo, foo_val] = value_map.at(0);
  auto [bar, bar_val] = value_map.at(1);
  expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i32Value", 1);
  expectWrappedValue(*bar_val, t_const_value::CV_INTEGER, "i16Value", 2);
}

std::unique_ptr<t_struct> make_foo_map(
    const t_program* program, const t_map* map_ty) {
  auto strct = std::make_unique<t_struct>(program, "FooMap");
  strct->append_field(
      std::make_unique<t_field>(&t_primitive_type::t_i32(), "foo", 1));
  strct->append_field(
      std::make_unique<t_field>(t_type_ref{*map_ty}, "foo_map", 2));
  return strct;
}

TEST(SchematizerTest, wrap_with_protocol_map) {
  // Resolution of nested properties can happen with a map field that does not
  // have a string-based key type.
  auto program = std::make_unique<t_program>("./", "./");
  auto map_ty = std::make_unique<t_map>(
      &t_primitive_type::t_i16(), &t_primitive_type::t_i32());
  auto foo_map_ty = make_foo_map(program.get(), map_ty.get());

  // Make the following structure:
  // FooMap {
  //   1: i32 foo = 1;
  //   2: map<i16, i32> foo_map = {
  //     444: 555,
  //     777: 888,
  //   }
  // };
  auto strct = t_const_value::make_map();
  strct->add_map(val("foo"), val(1));
  auto submap = t_const_value::make_map();
  submap->add_map(val(444), val(555));
  submap->add_map(val(777), val(888));
  strct->add_map(val("foo_map"), std::move(submap));

  auto value_no_type_mapping =
      protocol_value_builder{*foo_map_ty}.wrap(*strct, {});
  const auto& value_map =
      value_no_type_mapping->get_map().at(0).second->get_map();
  auto [foo, foo_val] = value_map.at(0);
  auto [foo_map, foo_map_val] = value_map.at(1);
  expectWrappedValue(*foo_val, t_const_value::CV_INTEGER, "i32Value", 1);

  const auto& foo_inner_map = foo_map_val->get_map().at(0).second->get_map();
  auto [inner_map_key, inner_map_val] = foo_inner_map.at(0);
  auto [inner_map_key2, inner_map_val2] = foo_inner_map.at(1);
  expectWrappedValue(
      *inner_map_key, t_const_value::CV_INTEGER, "i16Value", 444);
  expectWrappedValue(
      *inner_map_val, t_const_value::CV_INTEGER, "i32Value", 555);
  expectWrappedValue(
      *inner_map_key2, t_const_value::CV_INTEGER, "i16Value", 777);
  expectWrappedValue(
      *inner_map_val2, t_const_value::CV_INTEGER, "i32Value", 888);
}
