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

#include <thrift/compiler/gen/cpp/type_resolver.h>
#include <thrift/compiler/gen/testing.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <folly/portability/GTest.h>

#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/gen/cpp/namespace_resolver.h>

namespace apache::thrift::compiler::gen::cpp {
namespace {
// Since we can not include `thrift/annotation/cpp.thrift`
// This is a copy of apache::thrift::annotation::RefType
// The same copy exists in the reference_type.cc.
enum class RefType {
  Unique = 0,
  Shared = 1,
  SharedMutable = 2,
};

struct ref_builder : base_thrift_annotation_builder {
  explicit ref_builder(t_program& p, const std::string& lang)
      : base_thrift_annotation_builder(p, lang, "Ref") {}

  std::unique_ptr<t_const> make(const int64_t type) {
    auto map = std::make_unique<t_const_value>();
    map->set_map();
    map->add_map(make_string("type"), make_integer(type));
    return make_inst(std::move(map));
  }
};
} // namespace

class TypeResolverTest : public ::testing::Test {
 public:
  TypeResolverTest() noexcept
      : program_("path/to/program.thrift"), struct_(&program_, "ThriftStruct") {
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

  const std::string& get_storage_type(const t_field& node) {
    return resolver_.get_storage_type(node, struct_);
  }

  const std::string& get_reference_type(const t_field& node) {
    return resolver_.get_reference_type(node);
  }

  std::string get_type_tag(const t_type& type) {
    return resolver_.get_type_tag(type);
  }

  type_resolver resolver_;

 protected:
  t_program program_;
  t_struct struct_;
};

TEST_F(TypeResolverTest, BaseTypes) {
  EXPECT_EQ(get_native_type(t_base_type::t_void()), "void");
  EXPECT_EQ(get_native_type(t_base_type::t_bool()), "bool");
  EXPECT_EQ(get_native_type(t_base_type::t_byte()), "::std::int8_t");
  EXPECT_EQ(get_native_type(t_base_type::t_i16()), "::std::int16_t");
  EXPECT_EQ(get_native_type(t_base_type::t_i32()), "::std::int32_t");
  EXPECT_EQ(get_native_type(t_base_type::t_i64()), "::std::int64_t");
  EXPECT_EQ(get_native_type(t_base_type::t_float()), "float");
  EXPECT_EQ(get_native_type(t_base_type::t_double()), "double");
  EXPECT_EQ(get_native_type(t_base_type::t_string()), "::std::string");
  EXPECT_EQ(get_native_type(t_base_type::t_binary()), "::std::string");

  EXPECT_FALSE(can_resolve_to_scalar(t_base_type::t_void()));
  EXPECT_TRUE(can_resolve_to_scalar(t_base_type::t_bool()));
  EXPECT_TRUE(can_resolve_to_scalar(t_base_type::t_byte()));
  EXPECT_TRUE(can_resolve_to_scalar(t_base_type::t_i16()));
  EXPECT_TRUE(can_resolve_to_scalar(t_base_type::t_i32()));
  EXPECT_TRUE(can_resolve_to_scalar(t_base_type::t_i64()));
  EXPECT_TRUE(can_resolve_to_scalar(t_base_type::t_float()));
  EXPECT_TRUE(can_resolve_to_scalar(t_base_type::t_double()));
  EXPECT_FALSE(can_resolve_to_scalar(t_base_type::t_string()));
  EXPECT_FALSE(can_resolve_to_scalar(t_base_type::t_binary()));

  EXPECT_EQ(
      get_type_tag(t_base_type::t_void()), "::apache::thrift::type::void_t");
  EXPECT_EQ(
      get_type_tag(t_base_type::t_byte()), "::apache::thrift::type::byte_t");
  EXPECT_EQ(
      get_type_tag(t_base_type::t_string()),
      "::apache::thrift::type::string_t");
  EXPECT_EQ(
      get_type_tag(t_base_type::t_binary()),
      "::apache::thrift::type::binary_t");
  EXPECT_EQ(
      get_type_tag(t_base_type::t_i16()), "::apache::thrift::type::i16_t");
  EXPECT_EQ(
      get_type_tag(t_base_type::t_i32()), "::apache::thrift::type::i32_t");
  EXPECT_EQ(
      get_type_tag(t_base_type::t_i64()), "::apache::thrift::type::i64_t");
  EXPECT_EQ(
      get_type_tag(t_base_type::t_float()), "::apache::thrift::type::float_t");
  EXPECT_EQ(
      get_type_tag(t_base_type::t_double()),
      "::apache::thrift::type::double_t");
}

TEST_F(TypeResolverTest, Struct_Adapter) {
  t_struct strct(&program_, "Foo");
  strct.add_structured_annotation(
      adapter_builder(program_, "cpp").make("FooAdapter"));
  // The standard type is the default.
  EXPECT_EQ(get_standard_type(strct), "::path::to::detail::Foo");
  // The c++ type is adapted.
  EXPECT_EQ(get_native_type(strct), "::path::to::Foo");

  // cpp.name overrides the 'default' standard type.
  t_struct strct2(&program_, "Foo");
  strct2.set_annotation("cpp.name", "Bar");
  strct2.add_structured_annotation(
      adapter_builder(program_, "cpp").make("FooAdapter"));
  EXPECT_EQ(get_standard_type(strct2), "::path::to::detail::Bar");
  EXPECT_EQ(get_native_type(strct2), "::path::to::Bar");

  // cpp.adapter could resolve to a scalar.
  EXPECT_TRUE(can_resolve_to_scalar(strct));
}

TEST_F(TypeResolverTest, CppName) {
  t_enum tenum(&program_, "MyEnum");
  tenum.set_annotation("cpp.name", "YourEnum");
  EXPECT_EQ(get_native_type(tenum), "::path::to::YourEnum");
}

TEST_F(TypeResolverTest, Containers) {
  // A container could not resolve to a scalar.
  t_map tmap(t_base_type::t_string(), t_base_type::t_i32());
  EXPECT_EQ(get_native_type(tmap), "::std::map<::std::string, ::std::int32_t>");
  EXPECT_FALSE(can_resolve_to_scalar(tmap));
  t_list tlist(t_base_type::t_double());
  EXPECT_EQ(get_native_type(tlist), "::std::vector<double>");
  EXPECT_FALSE(can_resolve_to_scalar(tlist));
  t_set tset(tmap);
  EXPECT_EQ(
      get_native_type(tset),
      "::std::set<::std::map<::std::string, ::std::int32_t>>");
  EXPECT_FALSE(can_resolve_to_scalar(tset));
}

TEST_F(TypeResolverTest, Containers_CustomTemplate) {
  // cpp.template could not resolve to a scalar since it can be only used for
  // container fields.
  t_map tmap(t_base_type::t_string(), t_base_type::t_i32());
  tmap.set_annotation("cpp.template", "std::unordered_map");
  EXPECT_EQ(
      get_native_type(tmap),
      "std::unordered_map<::std::string, ::std::int32_t>");
  EXPECT_FALSE(can_resolve_to_scalar(tmap));
  t_list tlist(t_base_type::t_double());
  tlist.set_annotation("cpp2.template", "std::list");
  EXPECT_EQ(get_native_type(tlist), "std::list<double>");
  EXPECT_FALSE(can_resolve_to_scalar(tlist));
  t_set tset(t_base_type::t_binary());
  tset.set_annotation("cpp2.template", "::std::unordered_set");
  EXPECT_EQ(get_native_type(tset), "::std::unordered_set<::std::string>");
  EXPECT_FALSE(can_resolve_to_scalar(tset));
}

TEST_F(TypeResolverTest, Containers_Adapter) {
  // cpp.adapter could resolve to a scalar.
  t_struct strct(&program_, "Foo");
  strct.add_structured_annotation(
      adapter_builder(program_, "cpp").make("HashAdapter"));
  EXPECT_TRUE(can_resolve_to_scalar(strct));

  // Adapters work on container type arguments.
  t_map tmap(t_base_type::t_i16(), strct);
  EXPECT_EQ(
      get_standard_type(tmap),
      "::std::map<::std::int16_t, ::path::to::detail::Foo>");
  EXPECT_EQ(
      get_native_type(tmap), "::std::map<::std::int16_t, ::path::to::Foo>");

  // The container can also be addapted.
  t_set tset(strct);
  tset.set_annotation("cpp.template", "std::unordered_set");
  // The template argument is respected for both standard and adapted types.
  EXPECT_EQ(
      get_standard_type(tset), "std::unordered_set<::path::to::detail::Foo>");
  EXPECT_EQ(get_native_type(tset), "std::unordered_set<::path::to::Foo>");

  // cpp.type on the container overrides the 'default' standard type.
  t_list tlist(strct);
  tlist.set_annotation("cpp.type", "MyList");
  EXPECT_EQ(get_standard_type(tlist), "MyList");
  EXPECT_EQ(get_native_type(tlist), "MyList");
}

TEST_F(TypeResolverTest, Structs) {
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

TEST_F(TypeResolverTest, TypeDefs) {
  // Scalar
  t_typedef ttypedef1(&program_, "Foo", t_base_type::t_bool());
  EXPECT_EQ(get_native_type(ttypedef1), "::path::to::Foo");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef1));

  // Non-scalar
  t_typedef ttypedef2(&program_, "Foo", t_base_type::t_string());
  EXPECT_EQ(get_native_type(ttypedef2), "::path::to::Foo");
  EXPECT_FALSE(can_resolve_to_scalar(ttypedef2));
}

TEST_F(TypeResolverTest, TypeDefs_Nested) {
  // Scalar
  t_typedef ttypedef1(&program_, "Foo", t_base_type::t_bool());
  t_typedef ttypedef2(&program_, "Bar", ttypedef1);
  EXPECT_EQ(get_native_type(ttypedef2), "::path::to::Bar");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef1));
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef2));

  // Non-scalar
  t_typedef ttypedef3(&program_, "Foo", t_base_type::t_string());
  t_typedef ttypedef4(&program_, "Bar", ttypedef3);
  EXPECT_EQ(get_native_type(ttypedef4), "::path::to::Bar");
  EXPECT_FALSE(can_resolve_to_scalar(ttypedef3));
  EXPECT_FALSE(can_resolve_to_scalar(ttypedef4));
}

TEST_F(TypeResolverTest, TypeDefs_Adapter) {
  t_struct strct(&program_, "Foo");
  strct.add_structured_annotation(
      adapter_builder(program_, "cpp").make("HashAdapter"));

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
      adapter_builder(program_, "cpp").make("TypeDefAdapter"));
  EXPECT_EQ(get_standard_type(ttypedef2), "::path::to::detail::Foo");
  EXPECT_EQ(get_native_type(ttypedef2), "::path::to::MyHash");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef2));
  ASSERT_TRUE(resolver_.find_first_adapter(ttypedef2));
  EXPECT_EQ(*resolver_.find_first_adapter(ttypedef2), "TypeDefAdapter");

  // Structured annotation
  t_base_type booll(t_base_type::t_bool());
  t_typedef typedef1(&program_, "MyBool", booll);
  typedef1.add_structured_annotation(
      adapter_builder(program_, "cpp").make("MyAdapter"));
  t_typedef typedef2(&program_, "DoubleBool", typedef1);
  ASSERT_TRUE(resolver_.find_first_adapter(typedef2));
  EXPECT_EQ(*resolver_.find_first_adapter(typedef2), "MyAdapter");
}

TEST_F(TypeResolverTest, CustomType) {
  t_base_type tui64(t_base_type::t_i64());
  tui64.set_name("ui64");
  tui64.set_annotation("cpp2.type", "::std::uint64_t");
  EXPECT_EQ(get_native_type(tui64), "::std::uint64_t");
  EXPECT_TRUE(can_resolve_to_scalar(tui64));

  t_union tunion(&program_, "Bar");
  tunion.set_annotation("cpp2.type", "Other");
  EXPECT_EQ(get_native_type(tunion), "Other");
  EXPECT_TRUE(can_resolve_to_scalar(tunion));

  t_typedef ttypedef1(&program_, "Foo", t_base_type::t_bool());
  ttypedef1.set_annotation("cpp2.type", "Other");
  EXPECT_EQ(get_native_type(ttypedef1), "Other");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef1));

  t_typedef ttypedef2(&program_, "FooBar", t_base_type::t_string());
  ttypedef2.set_annotation("cpp2.type", "Other");
  EXPECT_EQ(get_native_type(ttypedef2), "Other");
  EXPECT_TRUE(can_resolve_to_scalar(ttypedef2));

  t_map tmap1(t_base_type::t_string(), tui64);
  EXPECT_EQ(
      get_native_type(tmap1), "::std::map<::std::string, ::std::uint64_t>");
  EXPECT_FALSE(can_resolve_to_scalar(tmap1));

  // Can be combined with template.
  t_map tmap2(tmap1);
  tmap2.set_annotation("cpp.template", "std::unordered_map");
  EXPECT_EQ(
      get_native_type(tmap2),
      "std::unordered_map<::std::string, ::std::uint64_t>");
  EXPECT_FALSE(can_resolve_to_scalar(tmap2));

  // Custom type overrides template.
  t_map tmap3(tmap2);
  tmap3.set_annotation("cpp.type", "MyMap");
  EXPECT_EQ(get_native_type(tmap3), "MyMap");
  EXPECT_TRUE(can_resolve_to_scalar(tmap3));
}

TEST_F(TypeResolverTest, Stream) {
  t_base_type ui64(t_base_type::t_i64());
  ui64.set_annotation("cpp.type", "uint64_t");

  auto fun1 = t_function(nullptr, {}, "", {}, std::make_unique<t_stream>(ui64));
  EXPECT_EQ(
      resolver_.get_return_type(fun1),
      "::apache::thrift::ServerStream<uint64_t>");

  auto fun2 = t_function(
      nullptr, t_type_ref(ui64), "", {}, std::make_unique<t_stream>(ui64));
  EXPECT_EQ(
      resolver_.get_return_type(fun2),
      "::apache::thrift::ResponseAndServerStream<uint64_t, uint64_t>");
}

TEST_F(TypeResolverTest, Sink) {
  t_struct tstruct(&program_, "Foo");

  auto fun1 = t_function(
      nullptr, {}, "", {}, std::make_unique<t_sink>(tstruct, tstruct));
  EXPECT_EQ(
      resolver_.get_return_type(fun1),
      "::apache::thrift::SinkConsumer<::path::to::Foo, ::path::to::Foo>");

  auto sink2 = std::make_unique<t_sink>(tstruct, tstruct);
  sink2->set_first_response_type(t_type_ref(tstruct));
  auto fun2 =
      t_function(nullptr, t_type_ref(tstruct), "", {}, std::move(sink2));
  EXPECT_EQ(
      resolver_.get_return_type(fun2),
      "::apache::thrift::ResponseAndSinkConsumer<"
      "::path::to::Foo, "
      "::path::to::Foo, "
      "::path::to::Foo>");
}

TEST_F(TypeResolverTest, StorageType) {
  t_struct strct(&program_, "Foo");
  strct.add_structured_annotation(
      adapter_builder(program_, "cpp").make("HashAdapter"));
  {
    t_field strct_field(strct, "hash", 1);
    EXPECT_EQ(get_storage_type(strct_field), "::path::to::Foo");
  }

  {
    t_field strct_field(strct, "hash", 1);
    strct_field.set_annotation("cpp.ref", "");
    EXPECT_EQ(
        get_storage_type(strct_field), "::std::unique_ptr<::path::to::Foo>");
  }
  {
    t_field strct_field(strct, "hash", 1);
    strct_field.set_annotation("cpp2.ref", ""); // Works with cpp2.
    EXPECT_EQ(
        get_storage_type(strct_field), "::std::unique_ptr<::path::to::Foo>");
  }

  {
    t_field strct_field(strct, "hash", 1);
    strct_field.set_annotation("cpp.ref_type", "unique");
    EXPECT_EQ(
        get_storage_type(strct_field), "::std::unique_ptr<::path::to::Foo>");
  }

  {
    t_field strct_field(strct, "hash", 1);
    strct_field.set_annotation("cpp2.ref_type", "shared"); // Works with cpp2.
    EXPECT_EQ(
        get_storage_type(strct_field), "::std::shared_ptr<::path::to::Foo>");
  }

  {
    t_field strct_field(strct, "hash", 1);
    strct_field.set_annotation("cpp.ref_type", "shared_mutable");
    EXPECT_EQ(
        get_storage_type(strct_field), "::std::shared_ptr<::path::to::Foo>");
  }

  {
    t_field strct_field(strct, "hash", 1);
    strct_field.set_annotation("cpp.ref_type", "shared_const");
    EXPECT_EQ(
        get_storage_type(strct_field),
        "::std::shared_ptr<const ::path::to::Foo>");
  }

  {
    t_field strct_field(strct, "hash", 1);
    strct_field.set_annotation("thrift.box", "");
    EXPECT_EQ(
        get_storage_type(strct_field),
        "::apache::thrift::detail::boxed_value_ptr<::path::to::Foo>");
  }

  {
    t_field strct_field(strct, "hash", 1);
    strct_field.add_structured_annotation(
        thrift_annotation_builder::box(program_).make());
    EXPECT_EQ(
        get_storage_type(strct_field),
        "::apache::thrift::detail::boxed_value_ptr<::path::to::Foo>");
  }

  { // Unrecognized throws an exception.
    t_field strct_field(strct, "hash", 1);
    strct_field.set_annotation("cpp.ref_type", "blah");
    EXPECT_THROW(get_storage_type(strct_field), std::runtime_error);
  }
}

TEST_F(TypeResolverTest, Typedef_cpptemplate) {
  // cpp.template could not resolve to a scalar since it can be only used for
  // container fields.
  t_map imap(t_base_type::t_i32(), t_base_type::t_string());
  t_typedef iumap(&program_, "iumap", imap);
  iumap.set_annotation("cpp.template", "std::unorderd_map");
  t_typedef tiumap(&program_, "tiumap", iumap);

  EXPECT_EQ(get_native_type(imap), "::std::map<::std::int32_t, ::std::string>");
  EXPECT_EQ(
      get_standard_type(imap), "::std::map<::std::int32_t, ::std::string>");
  EXPECT_FALSE(can_resolve_to_scalar(imap));

  // The 'cpp.template' annotations is applied to the typedef; however, the type
  // resolver only looks for it on container types.
  // TODO(afuller): Consider making the template annotation propagate through
  // the typedef.
  EXPECT_EQ(get_native_type(iumap), "::path::to::iumap");
  EXPECT_EQ(get_standard_type(iumap), "::path::to::iumap");
  EXPECT_FALSE(can_resolve_to_scalar(iumap));

  EXPECT_EQ(get_native_type(tiumap), "::path::to::tiumap");
  EXPECT_EQ(get_standard_type(tiumap), "::path::to::tiumap");
  EXPECT_FALSE(can_resolve_to_scalar(tiumap));
}

TEST_F(TypeResolverTest, Typedef_cpptype) {
  t_map imap(t_base_type::t_i32(), t_base_type::t_string());
  t_typedef iumap(&program_, "iumap", imap);
  iumap.set_annotation(
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

TEST_F(TypeResolverTest, AdaptedFieldType) {
  auto i64 = t_base_type::t_i64();
  auto field = t_field(i64, "n", 42);
  field.add_structured_annotation(
      adapter_builder(program_, "cpp").make("MyAdapter"));
  EXPECT_EQ(
      get_native_type(field),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>");
  EXPECT_EQ(
      get_storage_type(field),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>");
}

TEST_F(TypeResolverTest, AdaptedFieldStorageType) {
  auto i64 = t_base_type::t_i64();
  auto adapter = adapter_builder(program_, "cpp");
  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    EXPECT_EQ(
        get_storage_type(field),
        "::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>");
  }

  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.set_annotation("cpp.ref", "");
    EXPECT_EQ(
        get_storage_type(field),
        "::std::unique_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");
  }
  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.set_annotation("cpp2.ref", ""); // Works with cpp2.
    EXPECT_EQ(
        get_storage_type(field),
        "::std::unique_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");
  }

  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.set_annotation("cpp.ref_type", "unique");
    EXPECT_EQ(
        get_storage_type(field),
        "::std::unique_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");
  }

  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.set_annotation("cpp.ref_type", "shared");
    EXPECT_EQ(
        get_storage_type(field),
        "::std::shared_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");
  }

  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.set_annotation("cpp.ref_type", "shared_mutable");
    EXPECT_EQ(
        get_storage_type(field),
        "::std::shared_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");
  }

  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.set_annotation("cpp.ref_type", "shared_const");
    EXPECT_EQ(
        get_storage_type(field),
        "::std::shared_ptr<const ::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");
  }

  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.set_annotation("thrift.box", "");
    EXPECT_EQ(
        get_storage_type(field),
        "::apache::thrift::detail::boxed_value_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");
  }

  {
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.add_structured_annotation(
        thrift_annotation_builder::box(program_).make());
    EXPECT_EQ(
        get_storage_type(field),
        "::apache::thrift::detail::boxed_value_ptr<::apache::thrift::adapt_detail::adapted_field_t<"
        "MyAdapter, 42, ::std::int64_t, ThriftStruct>>");
  }

  { // Unrecognized throws an exception.
    auto field = t_field(i64, "n", 42);
    field.add_structured_annotation(adapter.make("MyAdapter"));
    field.set_annotation("cpp.ref_type", "blah");
    EXPECT_THROW(get_storage_type(field), std::runtime_error);
  }
}

TEST_F(TypeResolverTest, TransitivelyAdaptedFieldType) {
  auto annotation = t_struct(nullptr, "MyAnnotation");

  annotation.add_structured_annotation(
      adapter_builder(program_, "cpp").make("MyAdapter"));

  auto transitive = t_struct(nullptr, "Transitive");
  transitive.set_uri(kTransitiveUri);
  annotation.add_structured_annotation(
      std::make_unique<t_const>(&program_, &transitive, "", nullptr));

  auto i64 = t_base_type::t_i64();
  auto field1 = t_field(i64, "field1", 1);
  field1.add_structured_annotation(
      std::make_unique<t_const>(&program_, &annotation, "", nullptr));
  EXPECT_EQ(
      get_storage_type(field1),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 1, ::std::int64_t, ThriftStruct>");

  auto field2 = t_field(i64, "field2", 42);
  field2.add_structured_annotation(
      std::make_unique<t_const>(&program_, &annotation, "", nullptr));
  EXPECT_EQ(
      get_storage_type(field2),
      "::apache::thrift::adapt_detail::adapted_field_t<"
      "MyAdapter, 42, ::std::int64_t, ThriftStruct>");
}

TEST_F(TypeResolverTest, GenTypeTagContainer) {
  auto i16 = t_base_type::t_i16();
  auto i32 = t_base_type::t_i32();
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

TEST_F(TypeResolverTest, GenTypeTagStruct) {
  t_program p("path/to/program.thrift");
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

TEST_F(TypeResolverTest, BasicQualifier) {
  t_field default_i32 = t_field(t_base_type::t_i32(), "i32");
  EXPECT_EQ(get_reference_type(default_i32), "::apache::thrift::field_ref");

  t_field optional_i32 = t_field(t_base_type::t_i32(), "i32");
  optional_i32.set_req(t_field::e_req::optional);
  EXPECT_EQ(
      get_reference_type(optional_i32), "::apache::thrift::optional_field_ref");

  t_field terse_i32 = t_field(t_base_type::t_i32(), "i32");
  terse_i32.set_req(t_field::e_req::terse);
  EXPECT_EQ(get_reference_type(terse_i32), "::apache::thrift::terse_field_ref");
}

TEST_F(TypeResolverTest, HasReferenceTypeFalse) {
  {
    t_field field = t_field(t_base_type::t_i32(), "i32");
    field.add_structured_annotation(
        ref_builder(program_, "cpp").make((int)RefType::Unique));
  }
  {
    t_field field = t_field(t_base_type::t_i32(), "i32");
    field.add_structured_annotation(
        ref_builder(program_, "cpp").make((int)RefType::Shared));
  }
  {
    t_field field = t_field(t_base_type::t_i32(), "i32");
    field.add_structured_annotation(
        ref_builder(program_, "cpp").make((int)RefType::SharedMutable));
  }
}

} // namespace apache::thrift::compiler::gen::cpp
