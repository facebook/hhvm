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

#include <gtest/gtest.h>

#include <map>
#include <set>
#include <vector>

#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/References_types.h>
#include <thrift/test/gen-cpp2/References_types_custom_protocol.h>

using namespace apache::thrift;
using namespace apache::thrift::test;

namespace cpp2 {

template <typename ThriftStruct>
void create_reference_default_test(ThriftStruct& obj) {
  // Test that non-optional ref string fields are initialized.
  EXPECT_NE(nullptr, obj.def_field_ref());
  EXPECT_NE(nullptr, obj.req_field_ref());
  EXPECT_NE(nullptr, obj.def_unique_field_ref());
  EXPECT_NE(nullptr, obj.req_unique_field_ref());
  EXPECT_NE(nullptr, obj.def_shared_field_ref());
  EXPECT_NE(nullptr, obj.req_shared_field_ref());
  EXPECT_NE(nullptr, obj.def_shared_const_field_ref());
  EXPECT_NE(nullptr, obj.req_shared_const_field_ref());

  // Check that optional fields are absent from a default-constructed object.
  EXPECT_EQ(nullptr, obj.opt_field_ref());
  EXPECT_EQ(nullptr, obj.opt_unique_field_ref());
  EXPECT_EQ(nullptr, obj.opt_shared_field_ref());
  EXPECT_EQ(nullptr, obj.opt_shared_const_field_ref());
  EXPECT_FALSE(obj.opt_box_field_ref().has_value());
}

template <typename ThriftStruct, typename CppType>
void create_reference_clear_test(CppType value) {
  ThriftStruct obj;

  obj.def_field_ref() = std::make_unique<CppType>(value);
  obj.req_field_ref() = std::make_unique<CppType>(value);
  obj.opt_field_ref() = std::make_unique<CppType>(value);
  obj.def_unique_field_ref() = std::make_unique<CppType>(value);
  obj.req_unique_field_ref() = std::make_unique<CppType>(value);
  obj.opt_unique_field_ref() = std::make_unique<CppType>(value);
  obj.def_shared_field_ref() = std::make_shared<CppType>(value);
  obj.req_shared_field_ref() = std::make_shared<CppType>(value);
  obj.opt_shared_field_ref() = std::make_shared<CppType>(value);
  obj.def_shared_const_field_ref() = std::make_shared<CppType>(value);
  obj.req_shared_const_field_ref() = std::make_shared<CppType>(value);
  obj.opt_shared_const_field_ref() = std::make_shared<CppType>(value);
  obj.opt_box_field_ref() = value;

  EXPECT_EQ(*obj.def_field_ref(), value);
  EXPECT_EQ(*obj.req_field_ref(), value);
  EXPECT_EQ(*obj.opt_field_ref(), value);
  EXPECT_EQ(*obj.def_unique_field_ref(), value);
  EXPECT_EQ(*obj.req_unique_field_ref(), value);
  EXPECT_EQ(*obj.opt_unique_field_ref(), value);
  EXPECT_EQ(*obj.def_shared_field_ref(), value);
  EXPECT_EQ(*obj.req_shared_field_ref(), value);
  EXPECT_EQ(*obj.opt_shared_field_ref(), value);
  EXPECT_EQ(*obj.def_shared_const_field_ref(), value);
  EXPECT_EQ(*obj.req_shared_const_field_ref(), value);
  EXPECT_EQ(*obj.opt_shared_const_field_ref(), value);
  EXPECT_EQ(*obj.opt_box_field_ref(), value);

  apache::thrift::clear(obj);

  create_reference_default_test(obj);
}

template <typename ThriftStruct>
void create_adapted_reference_default_test(ThriftStruct& obj) {
  // Test that non-optional ref string fields are initialized.
  EXPECT_NE(nullptr, obj.def_shared_field_ref());
  EXPECT_NE(nullptr, obj.req_shared_field_ref());
  EXPECT_NE(nullptr, obj.def_shared_const_field_ref());
  EXPECT_NE(nullptr, obj.req_shared_const_field_ref());

  // Check that optional fields are absent from a default-constructed object.
  EXPECT_EQ(nullptr, obj.opt_shared_field_ref());
  EXPECT_EQ(nullptr, obj.opt_shared_const_field_ref());
  EXPECT_FALSE(obj.opt_box_field_ref().has_value());
}

TEST(References, recursive_ref_fields) {
  SimpleJSONProtocolWriter writer;
  folly::IOBufQueue buff;
  writer.setOutput(&buff, 1024);

  EXPECT_EQ(nullptr, buff.front());

  cpp2::RecursiveStruct a;
  // Normally non-optional fields are present in a default-constructed object,
  // here we check the special-case of a recursive data type with a non-optional
  // or even required reference to its own type: obviously this doesn't make a
  // lot of sense since any chain of such structure must either contain a cycle
  // (meaning we can't possibly serialize it) or a nullptr (meaning it's in fact
  // optional), but for historical reasons we allow this and default the
  // value to `nullptr`.
  EXPECT_EQ(nullptr, a.def_field().get());
  EXPECT_EQ(nullptr, a.req_field().get());
  // Check that optional fields are absent from a default-constructed object
  EXPECT_EQ(nullptr, a.opt_field().get());

  EXPECT_EQ(nullptr, a.def_field().get());
  EXPECT_EQ(nullptr, a.req_field().get());
  EXPECT_EQ(nullptr, a.opt_field().get());

  // this isn't the correct serialized size, but it's what simple json returns.
  // it is the correct length for a manually inspected, correct serializedSize
  EXPECT_EQ(120, a.serializedSize(&writer));
  EXPECT_EQ(120, a.serializedSizeZC(&writer));

  if (buff.front()) {
    EXPECT_EQ(0, buff.front()->length());
  }

  a.def_field() = std::make_unique<cpp2::RecursiveStruct>();
  a.opt_field() = std::make_unique<cpp2::RecursiveStruct>();
  EXPECT_EQ(415, a.serializedSize(&writer));
  EXPECT_EQ(415, a.serializedSizeZC(&writer));

  cpp2::RecursiveStruct b;
  b.def_field() = std::make_unique<cpp2::RecursiveStruct>();
  b.opt_field() = std::make_unique<cpp2::RecursiveStruct>();
  EXPECT_EQ(415, b.serializedSize(&writer));
  EXPECT_EQ(415, b.serializedSizeZC(&writer));
}

TEST(References, ref_struct_fields) {
  ReferringStruct obj;
  create_reference_default_test(obj);
}

TEST(References, ref_struct_fields_clear) {
  ReferringStruct obj;
  PlainStruct p;
  p.field() = 1;
  create_reference_clear_test<ReferringStruct>(p);
}

TEST(References, ref_struct_base_fields) {
  ReferringStructWithBaseTypeFields obj;
  create_reference_default_test(obj);
}

TEST(References, ref_struct_base_fields_clear) {
  ReferringStructWithBaseTypeFields obj;
  create_reference_clear_test<ReferringStructWithBaseTypeFields, int64_t>(1);
}

TEST(References, ref_struct_string_fields) {
  ReferringStructWithStringFields obj;
  create_reference_default_test(obj);
}

TEST(References, ref_struct_string_fields_clear) {
  ReferringStructWithStringFields obj;
  create_reference_clear_test<ReferringStructWithStringFields, std::string>(
      "1");
}

TEST(References, ref_struct_container_fields) {
  ReferringStructWithListFields list_obj;
  ReferringStructWithSetFields set_obj;
  ReferringStructWithMapFields map_obj;
  create_reference_default_test(list_obj);
  create_reference_default_test(set_obj);
  create_reference_default_test(map_obj);
}

TEST(References, ref_struct_container_fields_clear) {
  ReferringStructWithListFields list_obj;
  ReferringStructWithSetFields set_obj;
  ReferringStructWithMapFields map_obj;
  create_reference_clear_test<
      ReferringStructWithListFields,
      std::vector<int32_t>>({1});
  create_reference_clear_test<ReferringStructWithSetFields, std::set<int32_t>>(
      {1});
  create_reference_clear_test<
      ReferringStructWithMapFields,
      std::map<int32_t, int32_t>>({{1, 1}});
}

TEST(References, adapter_ref_struct_fields) {
  TypeAdapterRefStruct type_adapted_obj;
  FieldAdapterRefStruct field_adapted_obj;
  create_adapted_reference_default_test(type_adapted_obj);
  create_adapted_reference_default_test(field_adapted_obj);
}

TEST(References, type_adapter_ref_struct_fields_clear) {
  TypeAdapterRefStruct obj;
  Wrapper<std::string> wrapper{"1"};
  obj.def_shared_field() = std::make_shared<Wrapper<std::string>>(wrapper);
  obj.opt_shared_field() = std::make_shared<Wrapper<std::string>>(wrapper);
  obj.req_shared_field() = std::make_shared<Wrapper<std::string>>(wrapper);
  obj.def_shared_const_field() =
      std::make_shared<Wrapper<std::string>>(wrapper);
  obj.opt_shared_const_field() =
      std::make_shared<Wrapper<std::string>>(wrapper);
  obj.req_shared_const_field() =
      std::make_shared<Wrapper<std::string>>(wrapper);
  obj.opt_box_field() = Wrapper<std::string>{wrapper};

  EXPECT_EQ(obj.def_shared_field()->value, "1");
  EXPECT_EQ(obj.req_shared_field()->value, "1");
  EXPECT_EQ(obj.opt_shared_field()->value, "1");
  EXPECT_EQ(obj.def_shared_const_field()->value, "1");
  EXPECT_EQ(obj.req_shared_const_field()->value, "1");
  EXPECT_EQ(obj.opt_shared_const_field()->value, "1");
  EXPECT_EQ(obj.opt_box_field()->value, "1");

  apache::thrift::clear(obj);

  create_adapted_reference_default_test(obj);
}

TEST(References, field_adapter_ref_struct_fields_clear) {
  FieldAdapterRefStruct obj;

  // TODO(dokwon): Use Adapter::fromThriftField for explicitly initializing
  // adapted fields.
  EXPECT_EQ(obj.def_shared_field()->meta, &*obj.meta());
  EXPECT_FALSE(obj.opt_shared_field());
  EXPECT_EQ(obj.req_shared_field()->meta, &*obj.meta());
  // EXPECT_EQ(obj.def_shared_const_field_ref()->meta, &*obj.meta());
  // EXPECT_EQ(obj.req_shared_const_field_ref()->meta, &*obj.meta());
  EXPECT_FALSE(obj.opt_shared_const_field());
  EXPECT_FALSE(obj.opt_box_field());

  obj.def_shared_field() = std::make_shared<
      AdaptedWithContext<std::string, FieldAdapterRefStruct, 1>>("1");
  obj.opt_shared_field() = std::make_shared<
      AdaptedWithContext<std::string, FieldAdapterRefStruct, 2>>("1");
  obj.req_shared_field() = std::make_shared<
      AdaptedWithContext<std::string, FieldAdapterRefStruct, 3>>("1");
  obj.def_shared_const_field() = std::make_shared<
      AdaptedWithContext<std::string, FieldAdapterRefStruct, 4>>("1");
  obj.opt_shared_const_field() = std::make_shared<
      AdaptedWithContext<std::string, FieldAdapterRefStruct, 5>>("1");
  obj.req_shared_const_field() = std::make_shared<
      AdaptedWithContext<std::string, FieldAdapterRefStruct, 6>>("1");
  obj.opt_box_field() =
      AdaptedWithContext<std::string, FieldAdapterRefStruct, 7>{"1"};

  EXPECT_EQ(obj.def_shared_field()->value, "1");
  EXPECT_EQ(obj.req_shared_field()->value, "1");
  EXPECT_EQ(obj.opt_shared_field()->value, "1");
  EXPECT_EQ(obj.def_shared_const_field()->value, "1");
  EXPECT_EQ(obj.req_shared_const_field()->value, "1");
  EXPECT_EQ(obj.opt_shared_const_field()->value, "1");
  EXPECT_EQ(obj.opt_box_field()->value, "1");

  apache::thrift::clear(obj);

  create_adapted_reference_default_test(obj);
}

TEST(References, field_ref) {
  cpp2::ReferringStruct a;

  static_assert(
      std::is_same_v<decltype(a.def_field()), std::unique_ptr<PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(std::move(a).def_field()),
                std::unique_ptr<PlainStruct>&&>);
  static_assert(std::is_same_v<
                decltype(std::as_const(a).def_field()),
                const std::unique_ptr<PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(std::move(std::as_const(a)).def_field()),
                const std::unique_ptr<PlainStruct>&&>);
  static_assert(std::is_same_v<
                decltype(a.def_shared_field()),
                std::shared_ptr<PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(std::move(a).def_shared_field()),
                std::shared_ptr<PlainStruct>&&>);
  static_assert(std::is_same_v<
                decltype(std::as_const(a).def_shared_field()),
                const std::shared_ptr<PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(std::move(std::as_const(a)).def_shared_field()),
                const std::shared_ptr<PlainStruct>&&>);
  static_assert(std::is_same_v<
                decltype(a.def_shared_const_field()),
                std::shared_ptr<const PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(std::move(a).def_shared_const_field()),
                std::shared_ptr<const PlainStruct>&&>);
  static_assert(std::is_same_v<
                decltype(std::as_const(a).def_shared_const_field()),
                const std::shared_ptr<const PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(std::move(std::as_const(a)).def_shared_const_field()),
                const std::shared_ptr<const PlainStruct>&&>);

  a.def_field() = std::make_unique<PlainStruct>();
  a.def_field()->field() = 10;
  auto x = std::move(a).def_field();
  EXPECT_EQ(x->field(), 10);
  EXPECT_FALSE(a.def_field());

  a.def_field() = std::make_unique<PlainStruct>();
  a.def_field()->field() = 20;
  auto y = std::move(a.def_field());
  EXPECT_EQ(y->field(), 20);
  EXPECT_FALSE(a.def_field());
}

TEST(References, intern_box_access) {
  StructuredAnnotation a, b;
  EXPECT_EQ(
      &*std::as_const(a).intern_box_field(),
      &*std::as_const(b).intern_box_field());
  EXPECT_NE(&*std::as_const(a).intern_box_field(), &*b.intern_box_field());
  EXPECT_NE(&*a.intern_box_field(), &*b.intern_box_field());

  // clear sets fill boxed intern field to the shared intrinsic default.
  apache::thrift::clear(a);
  apache::thrift::clear(b);
  EXPECT_EQ(
      &*std::as_const(a).intern_box_field(),
      &*std::as_const(b).intern_box_field());

  b.intern_box_field().emplace();
  // address does not match.
  EXPECT_NE(
      &*std::as_const(a).intern_box_field(),
      &*std::as_const(b).intern_box_field());

  // reset sets fill boxed intern field to the shared intrinsic default when the
  // field does not have custom default.
  b.intern_box_field().reset();
  // value should still be equal.
  EXPECT_EQ(
      *std::as_const(a).intern_box_field(),
      *std::as_const(b).intern_box_field());
  EXPECT_EQ(
      std::as_const(a).intern_box_field(), std::as_const(b).intern_box_field());
}

TEST(References, intern_box_empty) {
  TerseInternBox obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  // own the field.
  obj.intern_box_field().emplace();
  EXPECT_TRUE(apache::thrift::empty(obj));

  // explicitly set the inner field.
  obj.intern_box_field().value().field() = 0;
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST(References, structured_annotation) {
  StructuredAnnotation a;
  EXPECT_EQ(nullptr, a.opt_unique_field());
  EXPECT_EQ(nullptr, a.opt_shared_field());
  EXPECT_EQ(nullptr, a.opt_shared_mutable_field());
  EXPECT_FALSE(std::as_const(a).intern_box_field().is_set());
  static_assert(std::is_same_v<
                decltype(a.opt_unique_field()),
                std::unique_ptr<PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(a.opt_shared_field()),
                std::shared_ptr<const PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(a.opt_shared_mutable_field()),
                std::shared_ptr<PlainStruct>&>);
  static_assert(std::is_same_v<
                decltype(std::as_const(a).intern_box_field()),
                apache::thrift::intern_boxed_field_ref<
                    const apache::thrift::detail::boxed_value<PlainStruct>&>>);

  PlainStruct plain;
  plain.field() = 10;
  a.opt_unique_field() = std::make_unique<PlainStruct>(plain);
  plain.field() = 20;
  a.opt_shared_field() = std::make_shared<const PlainStruct>(plain);
  plain.field() = 30;
  a.opt_shared_mutable_field() = std::make_shared<PlainStruct>(plain);
  plain.field() = 40;
  a.intern_box_field() = plain;

  EXPECT_EQ(10, a.opt_unique_field()->field());
  EXPECT_EQ(20, a.opt_shared_field()->field());
  EXPECT_EQ(30, a.opt_shared_mutable_field()->field());
  EXPECT_EQ(40, a.intern_box_field()->field());

  auto data = CompactSerializer::serialize<std::string>(a);
  StructuredAnnotation b;
  CompactSerializer::deserialize(data, b);
  EXPECT_EQ(a, b);
}

TEST(References, string_ref) {
  StructWithString a;
  static_assert(std::is_same_v<
                decltype(a.def_unique_string_ref()),
                std::unique_ptr<std::string>&>);
  static_assert(std::is_same_v<
                decltype(a.def_shared_string_ref()),
                std::shared_ptr<std::string>&>);
  static_assert(std::is_same_v<
                decltype(a.def_shared_string_const_ref()),
                std::shared_ptr<const std::string>&>);
  EXPECT_EQ("...", *a.def_unique_string_ref());
  EXPECT_EQ("...", *a.def_shared_string_ref());
  EXPECT_EQ("...", *a.def_shared_string_const_ref());

  *a.def_unique_string_ref() = "a";
  *a.def_shared_string_ref() = "b";

  auto data = SimpleJSONSerializer::serialize<std::string>(a);
  StructWithString b;
  SimpleJSONSerializer::deserialize(data, b);
  EXPECT_EQ(a, b);
}

TEST(References, CppRefUnionLessThan) {
  auto check = [](ReferringUnionWithCppRef& smallerAddress,
                  ReferringUnionWithCppRef& largerAddress) {
    *smallerAddress.get_box_string() = "2";
    *largerAddress.get_box_string() = "1";

    EXPECT_LT(smallerAddress.get_box_string(), largerAddress.get_box_string());
    EXPECT_GT(
        *smallerAddress.get_box_string(), *largerAddress.get_box_string());
    EXPECT_GT(smallerAddress, largerAddress);
  };

  ReferringUnionWithCppRef a;
  ReferringUnionWithCppRef b;
  a.set_box_string("");
  b.set_box_string("");
  if (a.get_box_string() < b.get_box_string()) {
    check(a, b);
  } else {
    check(b, a);
  }
}

TEST(References, CppRefUnionSetterGetter) {
  ReferringUnionWithCppRef a;
  PlainStruct p;
  p.field() = 42;

  a.set_box_plain(p);

  EXPECT_THROW(a.get_box_string(), bad_field_access);
  EXPECT_THROW(a.get_box_self(), bad_field_access);
  EXPECT_EQ(a.get_box_plain()->field(), 42);

  a.set_box_string("foo");

  EXPECT_THROW(a.get_box_plain(), bad_field_access);
  EXPECT_THROW(a.get_box_self(), bad_field_access);
  EXPECT_EQ(*a.get_box_string(), "foo");

  ReferringUnionWithCppRef b;

  b.set_box_self(a);

  EXPECT_THROW(b.get_box_string(), bad_field_access);
  EXPECT_THROW(b.get_box_plain(), bad_field_access);
  EXPECT_THROW(*b.get_box_self()->get_box_plain(), bad_field_access);
  EXPECT_THROW(*b.get_box_self()->get_box_self(), bad_field_access);
  EXPECT_EQ(*b.get_box_self()->get_box_string(), "foo");
}

TEST(References, UnionFieldRef) {
  ReferringUnion a;
  PlainStruct p;
  p.field() = 42;

  a.box_plain() = p;

  EXPECT_FALSE(a.box_string());
  EXPECT_TRUE(a.box_plain());
  EXPECT_FALSE(a.box_self());
  EXPECT_EQ(a.box_plain()->field(), 42);
  EXPECT_THROW(a.box_string().value(), bad_field_access);

  a.box_string().emplace("foo");

  EXPECT_TRUE(a.box_string());
  EXPECT_FALSE(a.box_plain());
  EXPECT_FALSE(a.box_self());
  EXPECT_EQ(a.box_string(), "foo");

  ReferringUnion b;

  b.box_self() = a;

  EXPECT_FALSE(b.box_string());
  EXPECT_FALSE(b.box_plain());
  EXPECT_TRUE(b.box_self());
  EXPECT_TRUE(b.box_self()->box_string());
  EXPECT_FALSE(b.box_self()->box_plain());
  EXPECT_FALSE(b.box_self()->box_self());
  EXPECT_EQ(b.box_self()->box_string(), "foo");
}

TEST(References, UnionLessThan) {
  auto check = [](ReferringUnion& smallerAddress,
                  ReferringUnion& largerAddress) {
    smallerAddress.box_string() = "2";
    largerAddress.box_string() = "1";
    EXPECT_LT(&*smallerAddress.box_string(), &*largerAddress.box_string());
    EXPECT_GT(smallerAddress.box_string(), largerAddress.box_string());
    EXPECT_GT(smallerAddress, largerAddress);
  };

  ReferringUnion a;
  ReferringUnion b;
  a.box_string() = "";
  b.box_string() = "";
  if (&*a.box_string() < &*b.box_string()) {
    check(a, b);
  } else {
    check(b, a);
  }
}

TEST(References, StructAdapterRefStruct) {
  using apache::thrift::test::basic::DirectlyAdaptedStruct;
  StructAdapterRefStruct obj;
  DirectlyAdaptedStruct wrapper = {};
  wrapper.value.data() = 1;

  create_adapted_reference_default_test(obj);

  obj.def_shared_field() = std::make_shared<DirectlyAdaptedStruct>(wrapper);
  obj.opt_shared_field() = std::make_shared<DirectlyAdaptedStruct>(wrapper);
  obj.req_shared_field() = std::make_shared<DirectlyAdaptedStruct>(wrapper);
  obj.def_shared_const_field() =
      std::make_shared<DirectlyAdaptedStruct>(wrapper);
  obj.opt_shared_const_field() =
      std::make_shared<DirectlyAdaptedStruct>(wrapper);
  obj.req_shared_const_field() =
      std::make_shared<DirectlyAdaptedStruct>(wrapper);
  obj.opt_box_field() = DirectlyAdaptedStruct{wrapper};

  EXPECT_EQ(obj.def_shared_field()->value.data(), 1);
  EXPECT_EQ(obj.req_shared_field()->value.data(), 1);
  EXPECT_EQ(obj.opt_shared_field()->value.data(), 1);
  EXPECT_EQ(obj.def_shared_const_field()->value.data(), 1);
  EXPECT_EQ(obj.req_shared_const_field()->value.data(), 1);
  EXPECT_EQ(obj.opt_shared_const_field()->value.data(), 1);
  EXPECT_EQ(obj.opt_box_field()->value.data(), 1);

  auto objs = CompactSerializer::serialize<std::string>(obj);
  StructAdapterRefStruct objd;
  CompactSerializer::deserialize(objs, objd);

  EXPECT_EQ(objd.def_shared_field()->value.data(), 1);
  EXPECT_EQ(objd.req_shared_field()->value.data(), 1);
  EXPECT_EQ(objd.opt_shared_field()->value.data(), 1);
  EXPECT_EQ(objd.def_shared_const_field()->value.data(), 1);
  EXPECT_EQ(objd.req_shared_const_field()->value.data(), 1);
  EXPECT_EQ(objd.opt_shared_const_field()->value.data(), 1);
  EXPECT_EQ(objd.opt_box_field()->value.data(), 1);
}

TEST(References, DoubleAdaptedRefStruct) {
  using apache::thrift::test::basic::DirectlyAdaptedStruct;
  DoubleAdaptedRefStruct obj;
  Wrapper<DirectlyAdaptedStruct> wrapper = {};
  wrapper.value.value.data() = 1;

  create_adapted_reference_default_test(obj);

  obj.def_shared_field() =
      std::make_shared<Wrapper<DirectlyAdaptedStruct>>(wrapper);
  obj.opt_shared_field() =
      std::make_shared<Wrapper<DirectlyAdaptedStruct>>(wrapper);
  obj.req_shared_field() =
      std::make_shared<Wrapper<DirectlyAdaptedStruct>>(wrapper);
  obj.def_shared_const_field() =
      std::make_shared<Wrapper<DirectlyAdaptedStruct>>(wrapper);
  obj.opt_shared_const_field() =
      std::make_shared<Wrapper<DirectlyAdaptedStruct>>(wrapper);
  obj.req_shared_const_field() =
      std::make_shared<Wrapper<DirectlyAdaptedStruct>>(wrapper);
  obj.opt_box_field() = wrapper;

  EXPECT_EQ(obj.def_shared_field()->value.value.data(), 1);
  EXPECT_EQ(obj.req_shared_field()->value.value.data(), 1);
  EXPECT_EQ(obj.opt_shared_field()->value.value.data(), 1);
  EXPECT_EQ(obj.def_shared_const_field()->value.value.data(), 1);
  EXPECT_EQ(obj.req_shared_const_field()->value.value.data(), 1);
  EXPECT_EQ(obj.opt_shared_const_field()->value.value.data(), 1);
  EXPECT_EQ(obj.opt_box_field()->value.value.data(), 1);

  auto objs = CompactSerializer::serialize<std::string>(obj);
  DoubleAdaptedRefStruct objd;
  CompactSerializer::deserialize(objs, objd);

  EXPECT_EQ(objd.def_shared_field()->value.value.data(), 1);
  EXPECT_EQ(objd.req_shared_field()->value.value.data(), 1);
  EXPECT_EQ(objd.opt_shared_field()->value.value.data(), 1);
  EXPECT_EQ(objd.def_shared_const_field()->value.value.data(), 1);
  EXPECT_EQ(objd.req_shared_const_field()->value.value.data(), 1);
  EXPECT_EQ(objd.opt_shared_const_field()->value.value.data(), 1);
  EXPECT_EQ(objd.opt_box_field()->value.value.data(), 1);
}

TEST(References, NonTriviallyDestructibleUnion) {
  NonTriviallyDestructibleUnion obj;
  obj.int_field().ensure() = 1;
  auto objd = CompactSerializer::deserialize<NonTriviallyDestructibleUnion>(
      CompactSerializer::serialize<std::string>(obj));
  EXPECT_EQ(obj, objd);
}

TEST(References, NonTriviallyDestructibleUnionSetter) {
  NonTriviallyDestructibleUnion obj;
  obj.set_int_field(1);
  auto objd = CompactSerializer::deserialize<NonTriviallyDestructibleUnion>(
      CompactSerializer::serialize<std::string>(obj));
  EXPECT_EQ(obj, objd);
}

TEST(References, is_cpp_ref_field_optional) {
  using apache::thrift::field_id;
  using apache::thrift::detail::qualifier::is_cpp_ref_field_optional;
  bool def_field =
      is_cpp_ref_field_optional<cpp2::RecursiveStruct, field_id<1>>::value;
  bool opt_field =
      is_cpp_ref_field_optional<cpp2::RecursiveStruct, field_id<2>>::value;
  bool req_field =
      is_cpp_ref_field_optional<cpp2::RecursiveStruct, field_id<3>>::value;
  bool plain_field =
      is_cpp_ref_field_optional<cpp2::PlainStruct, field_id<1>>::value;
  EXPECT_FALSE(def_field);
  EXPECT_TRUE(opt_field);
  EXPECT_FALSE(req_field);
  EXPECT_FALSE(plain_field);
}

TEST(References, OpClearFieldReferences) {
  {
    // Note, this is identical to the current behavior of clearing fields in
    // apache::thrift::clear. We currently do not initialize non-optional
    // cpp.ref fields in the constructor, but serialize it to the default value
    // for the optimization.
    cpp2::RecursiveStruct obj, obj2;
    op::clear_field<op::get_field_tag<cpp2::RecursiveStruct, ident::def_field>>(
        obj.def_field(), obj);
    op::clear_field<op::get_field_tag<cpp2::RecursiveStruct, ident::opt_field>>(
        obj.opt_field(), obj);
    op::clear_field<op::get_field_tag<cpp2::RecursiveStruct, ident::req_field>>(
        obj.req_field(), obj);
    EXPECT_EQ(obj.def_field(), nullptr);
    EXPECT_EQ(obj.opt_field(), nullptr);
    EXPECT_EQ(obj.req_field(), nullptr);
    apache::thrift::clear(obj2);
    EXPECT_EQ(obj, obj2);
  }
  {
    auto populate_struct = []() {
      cpp2::RecursiveStruct obj;
      obj.def_field() = std::make_unique<cpp2::RecursiveStruct>();
      obj.opt_field() = std::make_unique<cpp2::RecursiveStruct>();
      obj.req_field() = std::make_unique<cpp2::RecursiveStruct>();
      return obj;
    };

    cpp2::RecursiveStruct obj{populate_struct()}, obj2{populate_struct()};
    op::clear_field<op::get_field_tag<cpp2::RecursiveStruct, ident::def_field>>(
        obj.def_field(), obj);
    op::clear_field<op::get_field_tag<cpp2::RecursiveStruct, ident::opt_field>>(
        obj.opt_field(), obj);
    op::clear_field<op::get_field_tag<cpp2::RecursiveStruct, ident::req_field>>(
        obj.req_field(), obj);
    EXPECT_EQ(*obj.def_field(), cpp2::RecursiveStruct{});
    EXPECT_EQ(obj.opt_field(), nullptr);
    EXPECT_EQ(*obj.req_field(), cpp2::RecursiveStruct{});
    apache::thrift::clear(obj2);
    EXPECT_EQ(obj, obj2);
  }
}

} // namespace cpp2
