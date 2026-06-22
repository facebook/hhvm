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
#include <thrift/compiler/generate/schema_populator.h>
#include <thrift/compiler/sema/schematizer.h>

#include <gtest/gtest.h>

using namespace apache::thrift::compiler;
using apache::thrift::FieldId;
using apache::thrift::compiler::detail::protocol_value_builder;
using apache::thrift::compiler::detail::schema_populator;
using apache::thrift::compiler::detail::schematizer;
namespace protocol = apache::thrift::protocol;
namespace type = apache::thrift::type;

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
  auto value = protocol_value_builder::as_value_type().wrap(str);
  EXPECT_TRUE(value.is_string());
  EXPECT_EQ(value.as_string(), "foo");
}

enum class MyEnum : std::uint16_t { Foo = 1, Bar = 2 };

const protocol::Value& mapAtString(
    const protocol::Value& value, std::string_view key) {
  for (const auto& [map_key, map_value] : value.as_map()) {
    if (map_key.is_string() && map_key.as_string() == key) {
      return map_value;
    }
  }
  throw std::out_of_range(std::string{key});
}

const protocol::Value& mapAtInt(const protocol::Value& value, int64_t key) {
  for (const auto& [map_key, map_value] : value.as_map()) {
    if ((map_key.is_i16() && map_key.as_i16() == key) ||
        (map_key.is_i32() && map_key.as_i32() == key) ||
        (map_key.is_i64() && map_key.as_i64() == key)) {
      return map_value;
    }
  }
  throw std::out_of_range(std::to_string(key));
}

template <typename T>
void expectWrappedValue(
    const protocol::Value& wrappedValue,
    std::string_view label,
    T literalValue) {
  if (label == "i16Value") {
    EXPECT_TRUE(wrappedValue.is_i16());
    EXPECT_EQ(literalValue, wrappedValue.as_i16());
  } else if (label == "i32Value") {
    EXPECT_TRUE(wrappedValue.is_i32());
    EXPECT_EQ(literalValue, wrappedValue.as_i32());
  } else if (label == "i64Value") {
    EXPECT_TRUE(wrappedValue.is_i64());
    EXPECT_EQ(literalValue, wrappedValue.as_i64());
  } else if (label == "floatValue") {
    EXPECT_TRUE(wrappedValue.is_float());
    EXPECT_FLOAT_EQ(literalValue, wrappedValue.as_float());
  } else if (label == "doubleValue") {
    EXPECT_TRUE(wrappedValue.is_double());
    EXPECT_EQ(literalValue, wrappedValue.as_double());
  } else if (label == "boolValue") {
    EXPECT_TRUE(wrappedValue.is_bool());
    EXPECT_EQ(literalValue, wrappedValue.as_bool());
  } else {
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
      protocol_value_builder::as_value_type().wrap(*strct);
  expectWrappedValue(mapAtString(value_no_type_mapping, "foo"), "i64Value", 1);
  expectWrappedValue(mapAtString(value_no_type_mapping, "bar"), "i64Value", 2);
  expectWrappedValue(
      mapAtString(value_no_type_mapping, "baz"), "doubleValue", 3.14);
  expectWrappedValue(
      mapAtString(value_no_type_mapping, "qux"), "doubleValue", 4.2);
}

std::unique_ptr<t_struct> make_foo_bar(const t_program* program) {
  auto struct_ty = std::make_unique<t_struct>(program, "FooBar");
  struct_ty->append_field(
      std::make_unique<t_field>(t_primitive_type::t_i32(), "foo", 1));
  struct_ty->append_field(
      std::make_unique<t_field>(t_primitive_type::t_i16(), "bar", 2));
  struct_ty->append_field(
      std::make_unique<t_field>(t_primitive_type::t_float(), "baz", 3));
  struct_ty->append_field(
      std::make_unique<t_field>(t_primitive_type::t_double(), "qux", 4));
  return struct_ty;
}

std::unique_ptr<t_const_value> make_foo_bar_value() {
  auto strct = t_const_value::make_map();
  strct->add_map(val("foo"), val(1));
  strct->add_map(val("bar"), val(2));
  strct->add_map(val("baz"), val(3.14));
  strct->add_map(val("qux"), val(4.2));
  return strct;
}

TEST(SchematizerTest, wrap_with_protocol_with_struct_ty) {
  auto strct = make_foo_bar_value();

  // With the type mapping, integers will be represented by their types in
  // FooBar.
  auto program = std::make_unique<t_program>("./", "./");
  auto foo_bar_ty = make_foo_bar(program.get());
  auto value_no_type_mapping =
      protocol_value_builder{*foo_bar_ty.get()}.wrap(*strct);
  expectWrappedValue(mapAtString(value_no_type_mapping, "foo"), "i32Value", 1);
  expectWrappedValue(mapAtString(value_no_type_mapping, "bar"), "i16Value", 2);
  expectWrappedValue(
      mapAtString(value_no_type_mapping, "baz"), "floatValue", 3.14);
  expectWrappedValue(
      mapAtString(value_no_type_mapping, "qux"), "doubleValue", 4.2);
}

std::unique_ptr<t_struct> make_foo_enum(
    const t_program* program, const t_enum* my_enum) {
  auto struct_ty = std::make_unique<t_struct>(program, "FooBar");
  struct_ty->append_field(std::make_unique<t_field>(*my_enum, "foo", 1));
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
      protocol_value_builder{*foo_enum_ty.get()}.wrap(*strct);
  expectWrappedValue(mapAtString(value_no_type_mapping, "foo"), "i32Value", 1);
}

std::unique_ptr<t_list> make_foo_bar_list(const t_struct* element) {
  return std::make_unique<t_list>(*element);
}

TEST(SchematizerTest, wrap_with_protocol_list) {
  auto program = std::make_unique<t_program>("./", "./");
  auto foo_bar_strct_ty = make_foo_bar(program.get());
  auto foo_bar_list_ty = make_foo_bar_list(foo_bar_strct_ty.get());
  auto struct_with_list_ty =
      std::make_unique<t_struct>(program.get(), "FooBarList");
  struct_with_list_ty->append_field(
      std::make_unique<t_field>(*foo_bar_list_ty, "foos", 1));

  auto my_list = t_const_value::make_list();
  auto foo_bar_elem = t_const_value::make_map();
  foo_bar_elem->add_map(val("foo"), val(1));
  foo_bar_elem->add_map(val("bar"), val(2));
  my_list->add_list(foo_bar_elem.get()->clone());
  my_list->add_list(std::move(foo_bar_elem));

  auto foos = t_const_value::make_map();
  foos->add_map(val("foos"), std::move(my_list));

  auto value_no_type_mapping =
      protocol_value_builder{*struct_with_list_ty}.wrap(*foos);
  const auto& foos_val = mapAtString(value_no_type_mapping, "foos");
  EXPECT_TRUE(foos_val.is_list());
  const auto& foos_list = foos_val.as_list();
  EXPECT_EQ(foos_list.size(), 2);

  {
    const auto& li_val = foos_list.at(0);
    expectWrappedValue(mapAtString(li_val, "foo"), "i32Value", 1);
    expectWrappedValue(mapAtString(li_val, "bar"), "i16Value", 2);
  }
  {
    const auto& li_val = foos_list.at(1);
    expectWrappedValue(mapAtString(li_val, "foo"), "i32Value", 1);
    expectWrappedValue(mapAtString(li_val, "bar"), "i16Value", 2);
  }
}

TEST(SchematizerTest, wrap_with_protocol_set) {
  auto program = std::make_unique<t_program>("./", "./");
  auto foo_bar_strct_ty = make_foo_bar(program.get());
  auto foo_bar_set_ty = std::make_unique<t_set>(*foo_bar_strct_ty);
  auto struct_with_set_ty =
      std::make_unique<t_struct>(program.get(), "FooBarSet");
  struct_with_set_ty->append_field(
      std::make_unique<t_field>(*foo_bar_set_ty, "foos", 1));

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
      protocol_value_builder{*struct_with_set_ty}.wrap(*foos);
  const auto& foos_val = mapAtString(value_no_type_mapping, "foos");
  EXPECT_TRUE(foos_val.is_set());
  const auto& foos_set = foos_val.as_set();
  EXPECT_EQ(foos_set.size(), 2);
  bool saw_first = false;
  bool saw_second = false;
  for (const auto& set_val : foos_set) {
    if (mapAtString(set_val, "foo").as_i32() == 1) {
      expectWrappedValue(mapAtString(set_val, "bar"), "i16Value", 2);
      saw_first = true;
    } else if (mapAtString(set_val, "foo").as_i32() == 4) {
      expectWrappedValue(mapAtString(set_val, "bar"), "i16Value", 5);
      saw_second = true;
    }
  }
  EXPECT_TRUE(saw_first);
  EXPECT_TRUE(saw_second);
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
      protocol_value_builder{*foo_bar_typedef}.wrap(*strct);
  expectWrappedValue(mapAtString(value_no_type_mapping, "foo"), "i32Value", 1);
  expectWrappedValue(mapAtString(value_no_type_mapping, "bar"), "i16Value", 2);
}

std::unique_ptr<t_struct> make_foo_map(
    const t_program* program, const t_map* map_ty) {
  auto strct = std::make_unique<t_struct>(program, "FooMap");
  strct->append_field(
      std::make_unique<t_field>(t_primitive_type::t_i32(), "foo", 1));
  strct->append_field(
      std::make_unique<t_field>(t_type_ref{*map_ty}, "foo_map", 2));
  return strct;
}

TEST(SchematizerTest, wrap_with_protocol_map) {
  // Resolution of nested properties can happen with a map field that does not
  // have a string-based key type.
  auto program = std::make_unique<t_program>("./", "./");
  auto map_ty = std::make_unique<t_map>(
      t_primitive_type::t_i16(), t_primitive_type::t_i32());
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

  auto value_no_type_mapping = protocol_value_builder{*foo_map_ty}.wrap(*strct);
  expectWrappedValue(mapAtString(value_no_type_mapping, "foo"), "i32Value", 1);

  const auto& foo_map_val = mapAtString(value_no_type_mapping, "foo_map");
  expectWrappedValue(mapAtInt(foo_map_val, 444), "i32Value", 555);
  expectWrappedValue(mapAtInt(foo_map_val, 777), "i32Value", 888);
}

constexpr auto UTILITY_CLASSES = {
    std::pair(
        "StructuredAnnotation",
        "facebook.com/thrift/type/StructuredAnnotation"),
    std::pair("Annotation", "facebook.com/thrift/type/Annotation"),
    std::pair("ProtocolValue", "facebook.com/thrift/protocol/Value"),
    std::pair{"TypeUri", "facebook.com/thrift/type/TypeUri"},
};

template <typename F>
void with_schematizer(
    schematizer::options opts,
    schema_populator::intern_func intern_value,
    F&& f) {
  source_manager sm;
  sm.add_virtual_file("my_prog", "abcd");

  t_global_scope scope{};
  auto program = std::make_unique<t_program>("my_prog", "my_prog");

  // Add some well-known types to the scope.
  auto protocol_value_ty = std::make_unique<t_struct>(
      program.get(), "ProtocolValue"); // Can be empty
  protocol_value_ty->set_uri("facebook.com/thrift/protocol/Value");
  std::vector<std::unique_ptr<t_struct>> utility_types;
  for (const auto& [name, uri] : UTILITY_CLASSES) {
    utility_types.push_back(std::make_unique<t_struct>(program.get(), name));
    utility_types.back()->set_uri(uri);
    scope.add_def_by_uri(*utility_types.back().get());
  }

  schematizer schematizer{scope, sm, opts};
  schema_populator schema_populator{schematizer, std::move(intern_value)};
  f(*program.get(), schematizer, schema_populator);
}

template <typename F, typename G, typename H>
void with_structured_annotation_uris(
    const type::Struct& struct_schema,
    const std::string_view struct_name,
    F&& on_annotation,
    G&& on_structured_annotation,
    H&& on_annotation_by_key) {
  const auto& attrs = *struct_schema.attrs();
  EXPECT_EQ(*attrs.name(), struct_name);

  // Visit `structuredAnnotations`
  {
    const auto& structured_annotations = *attrs.structuredAnnotations();

    size_t i = 0;
    for (const auto annotation_id : structured_annotations) {
      on_structured_annotation(i, static_cast<size_t>(annotation_id));
      ++i;
    }
  }

  // Visit `annotations`
  {
    const auto& annotations = *attrs.annotations();

    size_t i = 0;
    for (const auto& [annotation_name, _] : annotations) {
      on_annotation(i, annotation_name);
      ++i;
    }
  }

  // Visit `annotionsByKey`
  {
    const auto& annotations = *attrs.annotationsByKey();

    size_t i = 0;
    for (const auto& [annotation_def_key, _] : annotations) {
      on_annotation_by_key(i, std::string{annotation_def_key});
      ++i;
    }
  }
}

template <typename F>
void with_foo_bar_plus_structured_annotations(t_program& program, F f) {
  auto foo_bar_with_annotations_ty =
      std::make_unique<t_struct>(&program, "FooBarWithStructuredAnnotations");
  auto foo_bar_ty = make_foo_bar(&program);
  foo_bar_with_annotations_ty->add_structured_annotation(
      std::make_unique<t_const>(
          &program,
          t_type_ref{*foo_bar_ty},
          "FooBarWithStructuredAnnotations",
          make_foo_bar_value()));

  f(*foo_bar_with_annotations_ty, *foo_bar_ty);
}

TEST(SchematizerTest, use_hash_for_structured_annotations) {
  for (const bool use_hash : {true, false}) {
    schematizer::options opts;
    opts.use_hash = use_hash;
    size_t id = 0;
    std::vector<protocol::Value> interned_values;
    auto intern_value = [&](protocol::Value val, t_program*) {
      interned_values.push_back(std::move(val));
      return static_cast<schematizer::value_id>(id++);
    };

    with_schematizer(
        opts,
        std::move(intern_value),
        [&](t_program& program,
            schematizer& schematizer,
            schema_populator& schema_populator) {
          with_foo_bar_plus_structured_annotations(
              program,
              [&](const auto& foo_bar_with_annotations_ty,
                  const auto& foo_bar_ty) {
                // Generate a schema.thrift representation of
                // `FooBarWithStructuredAnnotations`
                const type::Struct struct_schema =
                    schema_populator.gen_schema(foo_bar_with_annotations_ty);

                const auto on_annotation =
                    [&](size_t annotation_idx,
                        const std::string& annotation_name) {
                      ASSERT_EQ(
                          annotation_idx, 0); // There's only one annotation
                      EXPECT_EQ(
                          annotation_name,
                          "my_prog.FooBar"); // This is never hashed
                    };

                const auto on_structured_annotation = [&](size_t annotation_idx,
                                                          size_t id) {
                  ASSERT_EQ(
                      annotation_idx,
                      0); // There's only one structured annotation

                  const auto& interned_annotation =
                      interned_values.at(id).as_object();
                  ASSERT_TRUE(interned_annotation.contains(FieldId{1}));
                  const auto& type_uri =
                      interned_annotation.at(FieldId{1}).as_object();

                  // TypeUri should be a union, so check for the appropriate
                  // field
                  if (use_hash) {
                    ASSERT_TRUE(type_uri.contains(FieldId{4}));
                    EXPECT_EQ(
                        type_uri.at(FieldId{4}).as_binary().to<std::string>(),
                        schematizer.identify_definition(foo_bar_ty));
                  } else {
                    ASSERT_TRUE(type_uri.contains(FieldId{3}));
                    EXPECT_EQ(
                        type_uri.at(FieldId{3}).as_string(),
                        "my_prog.FooBar"); // This should not be hashed
                  }
                };

                const auto on_annotation_by_key =
                    [&](size_t annotation_idx,
                        const std::string& definition_key) {
                      ASSERT_EQ(
                          annotation_idx,
                          0); // There's only one annotation

                      // This is always hashed
                      EXPECT_TRUE(
                          definition_key.find("FooBar") == std::string::npos);
                      EXPECT_EQ(
                          definition_key,
                          schematizer.identify_definition(foo_bar_ty));
                    };
                // Verify that the URIs of the structured annotations are
                // hashed
                with_structured_annotation_uris(
                    struct_schema,
                    "FooBarWithStructuredAnnotations",
                    on_annotation,
                    on_structured_annotation,
                    on_annotation_by_key);
              });
        });
  }
}
