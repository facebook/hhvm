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

#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/test/gen_testing.h>

#include <memory>
#include <stdexcept>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>

using namespace apache::thrift::compiler;

namespace {
// Since we can not include `thrift/annotation/cpp.thrift`
// This is a copy of apache::thrift::annotation::RefType
// The same copy exists in the reference_type.cc.
enum class RefType {
  Unique = 0,
  Shared = 1,
  SharedMutable = 2,
};

struct ref_builder : gen::base_thrift_annotation_builder {
  explicit ref_builder(t_program& p, const std::string& lang)
      : base_thrift_annotation_builder(p, lang, "Ref") {}

  std::unique_ptr<t_const> make(const int64_t type) {
    auto map = t_const_value::make_map();
    map->add_map(make_string("type"), make_integer(type));
    return make_inst(std::move(map));
  }
};
} // namespace

class CppNameResolverTest : public ::testing::Test {
 public:
  CppNameResolverTest() noexcept
      : program_("path/to/program.thrift", "path/to/program.thrift"),
        struct_(&program_, "ThriftStruct") {
    program_.set_namespace("cpp2", "path.to");
  }

  bool can_resolve_to_scalar(const t_type& node) {
    return resolver_.can_resolve_to_scalar(node);
  }

  const std::string& get_native_type(const t_type& node) {
    return resolver_.get_native_type(node);
  }

  const std::string& get_native_type(const t_field& node) {
    return resolver_.get_native_type(node, struct_);
  }

  const std::string& get_standard_type(const t_type& node) {
    return resolver_.get_standard_type(node);
  }

  const std::string& get_standard_type(const t_field& node) {
    return resolver_.get_standard_type(node);
  }

  const std::string& get_storage_type(const t_field& node) {
    return resolver_.get_storage_type(node, struct_);
  }

  const std::string& get_reference_type(const t_field& node) {
    return resolver_.get_reference_type(node);
  }

  std::string get_type_tag(const t_type& type) {
    return resolver_.get_type_tag(type);
  }

  cpp_name_resolver resolver_;

 protected:
  t_program program_;
  t_struct struct_;
};

TEST_F(CppNameResolverTest, gen_namespace_components_cpp2) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  p.set_namespace("cpp2", "foo.bar");
  p.set_namespace("cpp", "baz.foo");
  EXPECT_THAT(
      cpp_name_resolver::gen_namespace_components(p),
      testing::ElementsAreArray({"foo", "bar"}));
}

TEST_F(CppNameResolverTest, gen_namespace_components_cpp) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  p.set_namespace("cpp", "baz.foo");
  EXPECT_THAT(
      cpp_name_resolver::gen_namespace_components(p),
      testing::ElementsAreArray({"baz", "foo", "cpp2"}));
}

TEST_F(CppNameResolverTest, gen_namespace_components_none) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  EXPECT_THAT(
      cpp_name_resolver::gen_namespace_components(p),
      testing::ElementsAreArray({"cpp2"}));
}

TEST_F(CppNameResolverTest, get_namespace_cpp2) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  p.set_namespace("cpp2", "foo.bar");
  p.set_namespace("cpp", "baz.foo");
  EXPECT_EQ("::foo::bar", resolver_.get_namespace(p));
}

TEST_F(CppNameResolverTest, get_namespace_cpp) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  p.set_namespace("cpp", "baz.foo");
  EXPECT_EQ("::baz::foo::cpp2", resolver_.get_namespace(p));
}

TEST_F(CppNameResolverTest, get_namespace_none) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  EXPECT_EQ("::cpp2", resolver_.get_namespace(p));
}

TEST_F(CppNameResolverTest, get_namespace_from_package) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  p.set_package(t_package("foo.bar/path/to/program"));
  EXPECT_EQ("::foo::path::to::program", resolver_.get_namespace(p));
}

TEST_F(CppNameResolverTest, gen_namespaced_name) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  p.set_namespace("cpp2", "foo.bar");
  t_enum e1(&p, "MyEnum1");
  t_enum e2(&p, "MyEnum2");
  e2.set_unstructured_annotation("cpp.name", "YourEnum");
  EXPECT_EQ("::foo::bar::MyEnum1", resolver_.get_namespaced_name(e1));
  EXPECT_EQ("::foo::bar::YourEnum", resolver_.get_namespaced_name(e2));
}

TEST_F(CppNameResolverTest, base_types) {
  EXPECT_EQ(get_native_type(t_primitive_type::t_void()), "void");
  EXPECT_EQ(get_native_type(t_primitive_type::t_bool()), "bool");
  EXPECT_EQ(get_native_type(t_primitive_type::t_byte()), "::std::int8_t");
  EXPECT_EQ(get_native_type(t_primitive_type::t_i16()), "::std::int16_t");
  EXPECT_EQ(get_native_type(t_primitive_type::t_i32()), "::std::int32_t");
  EXPECT_EQ(get_native_type(t_primitive_type::t_i64()), "::std::int64_t");
  EXPECT_EQ(get_native_type(t_primitive_type::t_float()), "float");
  EXPECT_EQ(get_native_type(t_primitive_type::t_double()), "double");
  EXPECT_EQ(get_native_type(t_primitive_type::t_string()), "::std::string");
  EXPECT_EQ(get_native_type(t_primitive_type::t_binary()), "::std::string");

  EXPECT_FALSE(can_resolve_to_scalar(t_primitive_type::t_void()));
  EXPECT_TRUE(can_resolve_to_scalar(t_primitive_type::t_bool()));
  EXPECT_TRUE(can_resolve_to_scalar(t_primitive_type::t_byte()));
  EXPECT_TRUE(can_resolve_to_scalar(t_primitive_type::t_i16()));
  EXPECT_TRUE(can_resolve_to_scalar(t_primitive_type::t_i32()));
  EXPECT_TRUE(can_resolve_to_scalar(t_primitive_type::t_i64()));
  EXPECT_TRUE(can_resolve_to_scalar(t_primitive_type::t_float()));
  EXPECT_TRUE(can_resolve_to_scalar(t_primitive_type::t_double()));
  EXPECT_FALSE(can_resolve_to_scalar(t_primitive_type::t_string()));
  EXPECT_FALSE(can_resolve_to_scalar(t_primitive_type::t_binary()));

  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_void()),
      "::apache::thrift::type::void_t");
  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_byte()),
      "::apache::thrift::type::byte_t");
  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_string()),
      "::apache::thrift::type::string_t");
  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_binary()),
      "::apache::thrift::type::binary_t");
  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_i16()), "::apache::thrift::type::i16_t");
  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_i32()), "::apache::thrift::type::i32_t");
  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_i64()), "::apache::thrift::type::i64_t");
  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_float()),
      "::apache::thrift::type::float_t");
  EXPECT_EQ(
      get_type_tag(t_primitive_type::t_double()),
      "::apache::thrift::type::double_t");
}

TEST_F(CppNameResolverTest, struct_adapter) {
  t_struct strct(&program_, "Foo");
  strct.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("FooAdapter"));
  // The standard type is the default.
  EXPECT_EQ(get_standard_type(strct), "::path::to::detail::Foo");
  // The c++ type is adapted.
  EXPECT_EQ(get_native_type(strct), "::path::to::Foo");

  // cpp.name overrides the 'default' standard type.
  t_struct strct2(&program_, "Foo");
  strct2.set_unstructured_annotation("cpp.name", "Bar");
  strct2.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("FooAdapter"));
  EXPECT_EQ(get_standard_type(strct2), "::path::to::detail::Bar");
  EXPECT_EQ(get_native_type(strct2), "::path::to::Bar");

  // cpp.adapter could resolve to a scalar.
  EXPECT_TRUE(can_resolve_to_scalar(strct));
}

TEST_F(CppNameResolverTest, cpp_name) {
  t_enum tenum(&program_, "MyEnum");
  tenum.set_unstructured_annotation("cpp.name", "YourEnum");
  EXPECT_EQ(get_native_type(tenum), "::path::to::YourEnum");
}

TEST_F(CppNameResolverTest, containers) {
  // A container could not resolve to a scalar.
  t_map tmap(t_primitive_type::t_string(), t_primitive_type::t_i32());
  EXPECT_EQ(get_native_type(tmap), "::std::map<::std::string, ::std::int32_t>");
  EXPECT_FALSE(can_resolve_to_scalar(tmap));
  t_list tlist(t_primitive_type::t_double());
  EXPECT_EQ(get_native_type(tlist), "::std::vector<double>");
  EXPECT_FALSE(can_resolve_to_scalar(tlist));
  t_set tset(tmap);
  EXPECT_EQ(
      get_native_type(tset),
      "::std::set<::std::map<::std::string, ::std::int32_t>>");
  EXPECT_FALSE(can_resolve_to_scalar(tset));
}

TEST_F(CppNameResolverTest, containers_custom_template) {
  // cpp.template could not resolve to a scalar since it can be only used for
  // container fields.
  t_map tmap(t_primitive_type::t_string(), t_primitive_type::t_i32());
  tmap.set_unstructured_annotation("cpp.template", "std::unordered_map");
  EXPECT_EQ(
      get_native_type(tmap),
      "std::unordered_map<::std::string, ::std::int32_t>");
  EXPECT_FALSE(can_resolve_to_scalar(tmap));
  t_list tlist(t_primitive_type::t_double());
  tlist.set_unstructured_annotation("cpp2.template", "std::list");
  EXPECT_EQ(get_native_type(tlist), "std::list<double>");
  EXPECT_FALSE(can_resolve_to_scalar(tlist));
  t_set tset(t_primitive_type::t_binary());
  tset.set_unstructured_annotation("cpp2.template", "::std::unordered_set");
  EXPECT_EQ(get_native_type(tset), "::std::unordered_set<::std::string>");
  EXPECT_FALSE(can_resolve_to_scalar(tset));
}

TEST_F(CppNameResolverTest, containers_adapter) {
  // cpp.adapter could resolve to a scalar.
  t_struct strct(&program_, "Foo");
  strct.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("HashAdapter"));
  EXPECT_TRUE(can_resolve_to_scalar(strct));

  // Adapters work on container type arguments.
  t_map tmap(t_primitive_type::t_i16(), strct);
  EXPECT_EQ(
      get_standard_type(tmap),
      "::std::map<::std::int16_t, ::path::to::detail::Foo>");
  EXPECT_EQ(
      get_native_type(tmap), "::std::map<::std::int16_t, ::path::to::Foo>");

  // The container can also be addapted.
  t_set tset(strct);
  tset.set_unstructured_annotation("cpp.template", "std::unordered_set");
  // The template argument is respected for both standard and adapted types.
  EXPECT_EQ(
      get_standard_type(tset), "std::unordered_set<::path::to::detail::Foo>");
  EXPECT_EQ(get_native_type(tset), "std::unordered_set<::path::to::Foo>");

  // cpp.type on the container overrides the 'default' standard type.
  t_list tlist(strct);
  tlist.set_unstructured_annotation("cpp.type", "MyList");
  EXPECT_EQ(get_standard_type(tlist), "MyList");
  EXPECT_EQ(get_native_type(tlist), "MyList");
}

TEST_F(CppNameResolverTest, structs) {
  t_struct tstruct(&program_, "Foo");
  EXPECT_EQ(get_native_type(tstruct), "::path::to::Foo");
  EXPECT_FALSE(can_resolve_to_scalar(tstruct));
  t_union tunion(&program_, "Bar");
  EXPECT_EQ(get_native_type(tunion), "::path::to::Bar");
  EXPECT_FALSE(can_resolve_to_scalar(tunion));
  t_exception texcept(&program_, "Baz");
  EXPECT_EQ(get_native_type(texcept), "::path::to::Baz");
  EXPECT_FALSE(can_resolve_to_scalar(texcept));
}

TEST_F(CppNameResolverTest, typedefs) {
  // Scalar
  t_typedef ttypedef1(&program_, "Foo", t_primitive_type::t_bool());
  EXPECT_EQ(get_native_type(ttypedef1), "::path::to::Foo");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef1));

  // Non-scalar
  t_typedef ttypedef2(&program_, "Foo", t_primitive_type::t_string());
  EXPECT_EQ(get_native_type(ttypedef2), "::path::to::Foo");
  EXPECT_FALSE(can_resolve_to_scalar(ttypedef2));
}

TEST_F(CppNameResolverTest, typedefs_nested) {
  // Scalar
  t_typedef ttypedef1(&program_, "Foo", t_primitive_type::t_bool());
  t_typedef ttypedef2(&program_, "Bar", ttypedef1);
  EXPECT_EQ(get_native_type(ttypedef2), "::path::to::Bar");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef1));
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef2));

  // Non-scalar
  t_typedef ttypedef3(&program_, "Foo", t_primitive_type::t_string());
  t_typedef ttypedef4(&program_, "Bar", ttypedef3);
  EXPECT_EQ(get_native_type(ttypedef4), "::path::to::Bar");
  EXPECT_FALSE(can_resolve_to_scalar(ttypedef3));
  EXPECT_FALSE(can_resolve_to_scalar(ttypedef4));
}

TEST_F(CppNameResolverTest, typedefs_adapter) {
  t_struct strct(&program_, "Foo");
  strct.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("HashAdapter"));

  // Type defs can refer to adatped types.
  t_typedef ttypedef1(&program_, "MyHash", strct);
  // It does not affect the type name.
  EXPECT_EQ(get_standard_type(ttypedef1), "::path::to::detail::Foo");
  EXPECT_EQ(get_native_type(ttypedef1), "::path::to::MyHash");
  // It is the refered to type that has the adapter.
  EXPECT_EQ(get_native_type(*ttypedef1.type()), "::path::to::Foo");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef1));
  ASSERT_TRUE(resolver_.find_first_adapter(ttypedef1));
  EXPECT_EQ(*resolver_.find_first_adapter(ttypedef1), "HashAdapter");

  // Type defs can also be adapted.
  t_typedef ttypedef2(ttypedef1);
  ttypedef2.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("TypeDefAdapter"));
  EXPECT_EQ(get_standard_type(ttypedef2), "::path::to::detail::Foo");
  EXPECT_EQ(get_native_type(ttypedef2), "::path::to::MyHash");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef2));
  ASSERT_TRUE(resolver_.find_first_adapter(ttypedef2));
  EXPECT_EQ(*resolver_.find_first_adapter(ttypedef2), "TypeDefAdapter");

  // Structured annotation
  t_primitive_type booll(t_primitive_type::t_bool());
  t_typedef typedef1(&program_, "MyBool", booll);
  typedef1.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("MyAdapter"));
  t_typedef typedef2(&program_, "DoubleBool", typedef1);
  ASSERT_TRUE(resolver_.find_first_adapter(typedef2));
  EXPECT_EQ(*resolver_.find_first_adapter(typedef2), "MyAdapter");
}

TEST_F(CppNameResolverTest, custom_type) {
  t_primitive_type tui64(t_primitive_type::t_i64());
  tui64.set_name("ui64");
  tui64.set_unstructured_annotation("cpp2.type", "::std::uint64_t");
  EXPECT_EQ(get_native_type(tui64), "::std::uint64_t");
  EXPECT_TRUE(can_resolve_to_scalar(tui64));

  t_union tunion(&program_, "Bar");
  tunion.set_unstructured_annotation("cpp2.type", "Other");
  EXPECT_EQ(get_native_type(tunion), "Other");
  EXPECT_TRUE(can_resolve_to_scalar(tunion));

  t_typedef ttypedef1(&program_, "Foo", t_primitive_type::t_bool());
  ttypedef1.set_unstructured_annotation("cpp2.type", "Other");
  EXPECT_EQ(get_native_type(ttypedef1), "Other");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef1));

  t_typedef ttypedef2(&program_, "FooBar", t_primitive_type::t_string());
  ttypedef2.set_unstructured_annotation("cpp2.type", "Other");
  EXPECT_EQ(get_native_type(ttypedef2), "Other");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef2));

  t_map tmap1(t_primitive_type::t_string(), tui64);
  EXPECT_EQ(
      get_native_type(tmap1), "::std::map<::std::string, ::std::uint64_t>");
  EXPECT_FALSE(can_resolve_to_scalar(tmap1));

  // Can be combined with template.
  t_map tmap2(tmap1);
  tmap2.set_unstructured_annotation("cpp.template", "std::unordered_map");
  EXPECT_EQ(
      get_native_type(tmap2),
      "std::unordered_map<::std::string, ::std::uint64_t>");
  EXPECT_FALSE(can_resolve_to_scalar(tmap2));

  // Custom type overrides template.
  t_map tmap3(tmap2);
  tmap3.set_unstructured_annotation("cpp.type", "MyMap");
  EXPECT_EQ(get_native_type(tmap3), "MyMap");
  EXPECT_TRUE(can_resolve_to_scalar(tmap3));
}

TEST_F(CppNameResolverTest, stream) {
  t_primitive_type ui64(t_primitive_type::t_i64());
  ui64.set_unstructured_annotation("cpp.type", "uint64_t");

  auto fun1 =
      t_function(nullptr, {}, "", {}, {}, std::make_unique<t_stream>(ui64));
  EXPECT_EQ(
      resolver_.get_return_type(fun1),
      "::apache::thrift::ServerStream<uint64_t>");

  auto fun2 = t_function(
      nullptr, t_type_ref(ui64), "", {}, {}, std::make_unique<t_stream>(ui64));
  EXPECT_EQ(
      resolver_.get_return_type(fun2),
      "::apache::thrift::ResponseAndServerStream<uint64_t, uint64_t>");
}

TEST_F(CppNameResolverTest, sink) {
  t_struct tstruct(&program_, "Foo");

  auto fun1 = t_function(
      nullptr, {}, "", {}, std::make_unique<t_sink>(tstruct, tstruct));
  EXPECT_EQ(
      resolver_.get_return_type(fun1),
      "::apache::thrift::SinkConsumer<::path::to::Foo, ::path::to::Foo>");

  auto sink2 = std::make_unique<t_sink>(tstruct, tstruct);
  auto fun2 =
      t_function(nullptr, t_type_ref(tstruct), "", {}, std::move(sink2));
  EXPECT_EQ(
      resolver_.get_return_type(fun2),
      "::apache::thrift::ResponseAndSinkConsumer<"
      "::path::to::Foo, "
      "::path::to::Foo, "
      "::path::to::Foo>");
}

TEST_F(CppNameResolverTest, storage_type) {
  t_struct strct(&program_, "Foo");
  auto cpp_ref = gen::cpp_ref_builder(program_);
  strct.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("HashAdapter"));

  t_field strct_field(strct, "hash", 1);
  EXPECT_EQ(get_storage_type(strct_field), "::path::to::Foo");

  t_field unique_strct_field(strct, "hash", 1);
  unique_strct_field.add_structured_annotation(cpp_ref.unique());
  EXPECT_EQ(
      get_storage_type(unique_strct_field),
      "::std::unique_ptr<::path::to::Foo>");

  t_field shared_mutable_strct_field(strct, "hash", 1);
  shared_mutable_strct_field.add_structured_annotation(
      cpp_ref.shared_mutable());
  EXPECT_EQ(
      get_storage_type(shared_mutable_strct_field),
      "::std::shared_ptr<::path::to::Foo>");

  t_field shared_strct_field(strct, "hash", 1);
  shared_strct_field.add_structured_annotation(cpp_ref.shared());
  EXPECT_EQ(
      get_storage_type(shared_strct_field),
      "::std::shared_ptr<const ::path::to::Foo>");

  t_field structured_box_strct_field(strct, "hash", 1);
  structured_box_strct_field.add_structured_annotation(
      gen::thrift_annotation_builder::box(program_).make());
  EXPECT_EQ(
      get_storage_type(structured_box_strct_field),
      "::apache::thrift::detail::boxed_value_ptr<::path::to::Foo>");

  // Unrecognized throws an exception.
  t_field unrecognized_strct_field(strct, "hash", 1);
  unrecognized_strct_field.set_unstructured_annotation("cpp.ref_type", "blah");
  EXPECT_THROW(get_storage_type(unrecognized_strct_field), std::runtime_error);
}

TEST_F(CppNameResolverTest, typedef_cpptemplate) {
  // cpp.template could not resolve to a scalar since it can be only used for
  // container fields.
  t_map imap(t_primitive_type::t_i32(), t_primitive_type::t_string());
  t_typedef iumap(&program_, "iumap", imap);
  iumap.set_unstructured_annotation("cpp.template", "std::unorderd_map");
  t_typedef tiumap(&program_, "tiumap", iumap);

  EXPECT_EQ(get_native_type(imap), "::std::map<::std::int32_t, ::std::string>");
  EXPECT_EQ(
      get_standard_type(imap), "::std::map<::std::int32_t, ::std::string>");
  EXPECT_FALSE(can_resolve_to_scalar(imap));

  EXPECT_EQ(
      get_native_type(iumap),
      "std::unorderd_map<::std::int32_t, ::std::string>");
  EXPECT_EQ(
      get_standard_type(iumap),
      "std::unorderd_map<::std::int32_t, ::std::string>");
  EXPECT_FALSE(can_resolve_to_scalar(iumap));

  EXPECT_EQ(get_native_type(tiumap), "::path::to::tiumap");
  EXPECT_EQ(get_standard_type(tiumap), "::path::to::tiumap");
  EXPECT_FALSE(can_resolve_to_scalar(tiumap));
}

TEST_F(CppNameResolverTest, typedef_cpptype) {
  t_map imap(t_primitive_type::t_i32(), t_primitive_type::t_string());
  t_typedef iumap(&program_, "iumap", imap);
  iumap.set_unstructured_annotation(
      "cpp.type", "std::unorderd_map<::std::int32_t, ::std::string>");
  t_typedef tiumap(&program_, "tiumap", iumap);

  EXPECT_EQ(get_native_type(imap), "::std::map<::std::int32_t, ::std::string>");
  EXPECT_EQ(
      get_standard_type(imap), "::std::map<::std::int32_t, ::std::string>");
  EXPECT_FALSE(can_resolve_to_scalar(imap));

  // The 'cpp.type' annotation is respected on the typedef.
  // TODO(afuller): It seems like this annotation is applied incorrectly and the
  // annotation should apply to the type being referenced, not the type name of
  // the typedef itself (which should just be the typedef's specified name)
  // which is still code-genend, but with the wrong type!
  EXPECT_EQ(
      get_native_type(iumap),
      "std::unorderd_map<::std::int32_t, ::std::string>");
  EXPECT_EQ(
      get_standard_type(iumap),
      "std::unorderd_map<::std::int32_t, ::std::string>");
  EXPECT_TRUE(can_resolve_to_scalar(iumap));

  EXPECT_EQ(get_native_type(tiumap), "::path::to::tiumap");
  EXPECT_EQ(get_standard_type(tiumap), "::path::to::tiumap");
  EXPECT_TRUE(can_resolve_to_scalar(tiumap));
}

TEST_F(CppNameResolverTest, adapted_field_type) {
  auto i64 = t_primitive_type::t_i64();
  auto field = t_field(i64, "n", 42);
  field.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("MyAdapter"));
  EXPECT_EQ(get_standard_type(field), "::std::int64_t");
  EXPECT_EQ(
      get_native_type(field),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>");
  EXPECT_EQ(
      get_storage_type(field),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>");
}

TEST_F(CppNameResolverTest, adapted_field_storage_type) {
  auto i64 = t_primitive_type::t_i64();
  auto cpp_ref = gen::cpp_ref_builder(program_);
  auto adapter = gen::adapter_builder(program_, "cpp");

  auto field = t_field(i64, "n", 42);
  field.add_structured_annotation(adapter.make("MyAdapter"));
  EXPECT_EQ(
      get_storage_type(field),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>");

  auto unique_field = t_field(i64, "n", 42);
  unique_field.add_structured_annotation(adapter.make("MyAdapter"));
  unique_field.add_structured_annotation(cpp_ref.unique());
  EXPECT_EQ(
      get_storage_type(unique_field),
      "::std::unique_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");

  auto shared_mutable_field = t_field(i64, "n", 42);
  shared_mutable_field.add_structured_annotation(adapter.make("MyAdapter"));
  shared_mutable_field.add_structured_annotation(cpp_ref.shared_mutable());
  EXPECT_EQ(
      get_storage_type(shared_mutable_field),
      "::std::shared_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");

  auto shared_field = t_field(i64, "n", 42);
  shared_field.add_structured_annotation(adapter.make("MyAdapter"));
  shared_field.add_structured_annotation(cpp_ref.shared());
  EXPECT_EQ(
      get_storage_type(shared_field),
      "::std::shared_ptr<const ::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");

  auto structured_box_field = t_field(i64, "n", 42);
  structured_box_field.add_structured_annotation(adapter.make("MyAdapter"));
  structured_box_field.add_structured_annotation(
      gen::thrift_annotation_builder::box(program_).make());
  EXPECT_EQ(
      get_storage_type(structured_box_field),
      "::apache::thrift::detail::boxed_value_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");

  // Unrecognized throws an exception.
  auto unrecognized_field = t_field(i64, "n", 42);
  unrecognized_field.add_structured_annotation(adapter.make("MyAdapter"));
  unrecognized_field.set_unstructured_annotation("cpp.ref_type", "blah");
  EXPECT_THROW(get_storage_type(unrecognized_field), std::runtime_error);
}

TEST_F(CppNameResolverTest, transitively_adapted_field_type) {
  auto annotation = t_struct(nullptr, "MyAnnotation");

  annotation.add_structured_annotation(
      gen::adapter_builder(program_, "cpp").make("MyAdapter"));

  auto transitive = t_struct(nullptr, "Transitive");
  transitive.set_uri(kTransitiveUri);
  annotation.add_structured_annotation(
      std::make_unique<t_const>(&program_, transitive, "", nullptr));

  auto i64 = t_primitive_type::t_i64();
  auto field1 = t_field(i64, "field1", 1);
  field1.add_structured_annotation(
      std::make_unique<t_const>(&program_, annotation, "", nullptr));
  EXPECT_EQ(
      get_storage_type(field1),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 1, ::std::int64_t, ThriftStruct>");

  auto field2 = t_field(i64, "field2", 42);
  field2.add_structured_annotation(
      std::make_unique<t_const>(&program_, annotation, "", nullptr));
  EXPECT_EQ(
      get_storage_type(field2),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>");
}

TEST_F(CppNameResolverTest, gen_type_tag_container) {
  auto i16 = t_primitive_type::t_i16();
  auto i32 = t_primitive_type::t_i32();
  t_list i32_list(i32);
  t_set i32_set(i32);
  t_map i32_i16_map(i32, i16);
  EXPECT_EQ(
      get_type_tag(i32_list),
      "::apache::thrift::type::list<::apache::thrift::type::i32_t>");
  EXPECT_EQ(
      get_type_tag(i32_set),
      "::apache::thrift::type::set<::apache::thrift::type::i32_t>");
  EXPECT_EQ(
      get_type_tag(i32_i16_map),
      "::apache::thrift::type::map<::apache::thrift::type::i32_t, ::apache::thrift::type::i16_t>");
}

TEST_F(CppNameResolverTest, gen_type_tag_struct) {
  t_program p("path/to/program.thrift", "path/to/program.thrift");
  t_struct s(&p, "struct_name");
  t_union u(&p, "union_name");
  t_exception e(&p, "exception_name");
  EXPECT_EQ(
      get_type_tag(s), "::apache::thrift::type::struct_t<::cpp2::struct_name>");
  EXPECT_EQ(
      get_type_tag(u), "::apache::thrift::type::union_t<::cpp2::union_name>");
  EXPECT_EQ(
      get_type_tag(e),
      "::apache::thrift::type::exception_t<::cpp2::exception_name>");
}

TEST_F(CppNameResolverTest, basic_qualifier) {
  t_field default_i32 = t_field(t_primitive_type::t_i32(), "i32");
  EXPECT_EQ(get_reference_type(default_i32), "::apache::thrift::field_ref");

  t_field optional_i32 = t_field(t_primitive_type::t_i32(), "i32");
  optional_i32.set_qualifier(t_field_qualifier::optional);
  EXPECT_EQ(
      get_reference_type(optional_i32), "::apache::thrift::optional_field_ref");

  t_field terse_i32 = t_field(t_primitive_type::t_i32(), "i32");
  terse_i32.set_qualifier(t_field_qualifier::terse);
  EXPECT_EQ(get_reference_type(terse_i32), "::apache::thrift::terse_field_ref");
}

TEST_F(CppNameResolverTest, has_reference_type_false) {
  {
    t_field field = t_field(t_primitive_type::t_i32(), "i32");
    field.add_structured_annotation(
        ref_builder(program_, "cpp").make((int)RefType::Unique));
  }
  {
    t_field field = t_field(t_primitive_type::t_i32(), "i32");
    field.add_structured_annotation(
        ref_builder(program_, "cpp").make((int)RefType::Shared));
  }
  {
    t_field field = t_field(t_primitive_type::t_i32(), "i32");
    field.add_structured_annotation(
        ref_builder(program_, "cpp").make((int)RefType::SharedMutable));
  }
}
