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

#include <memory>
#include <thrift/test/gen-cpp2/structs_terse_types.h>
#include <thrift/test/gen-cpp2/structs_types.h>

#include <folly/Traits.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using namespace apache::thrift::test;

namespace {
using apache::thrift::detail::st::struct_private_access;

class StructTest : public testing::Test {};

TEST_F(StructTest, compilation_terse_writes_refs_shared) {
  BasicRefsSharedTerseWrites a;
  (void)a;
}

TEST_F(StructTest, serialization_terse_writes_refs_shared) {
  using apache::thrift::CompactSerializer;

  BasicRefsSharedTerseWrites a;
  EXPECT_FALSE(apache::thrift::empty(a));

  a.shared_field_ref() = std::make_shared<HasInt>();
  a.shared_field_ref()->field() = 3;

  a.shared_fields_ref() = std::make_shared<std::vector<HasInt>>();
  a.shared_fields_ref()->emplace_back();
  a.shared_fields_ref()->back().field() = 4;
  a.shared_fields_ref()->emplace_back();
  a.shared_fields_ref()->back().field() = 5;
  a.shared_fields_ref()->emplace_back();
  a.shared_fields_ref()->back().field() = 6;

  a.shared_fields_const_ref() = std::make_shared<const std::vector<HasInt>>();

  const std::string serialized = CompactSerializer::serialize<std::string>(a);

  BasicRefsSharedTerseWrites b;
  CompactSerializer::deserialize(serialized, b);

  EXPECT_EQ(a, b);
}

TEST_F(StructTest, serialization_terse_writes_default_values) {
  using apache::thrift::CompactSerializer;

  BasicRefsSharedTerseWrites empty;

  BasicRefsSharedTerseWrites defaults;
  defaults.shared_field_req_ref() = std::make_shared<HasInt>();
  defaults.shared_fields_req_ref() = std::make_shared<std::vector<HasInt>>();

  // This struct has terse writes enabled, so the default values set above
  // should not be part of the serialization.
  EXPECT_EQ(
      CompactSerializer::serialize<std::string>(empty),
      CompactSerializer::serialize<std::string>(defaults));
}

TEST_F(StructTest, equal_to) {
  std::equal_to<Basic> op;

  {
    Basic a;
    Basic b;

    *a.def_field() = 3;
    *b.def_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field().ensure();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    b.def_field().ensure();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *a.def_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.def_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.opt_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.opt_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    *a.req_field() = 3;
    *b.req_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *a.req_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.req_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    a.req_field() = 3;
    b.req_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.req_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.req_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
}

// currently, binary fields are handled specially, so they get their own tests
TEST_F(StructTest, equal_to_binary) {
  std::equal_to<BasicBinaries> op;

  {
    BasicBinaries a;
    BasicBinaries b;

    *a.def_field() = "hello";
    *b.def_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field().ensure();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    b.def_field().ensure();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *a.def_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.def_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.opt_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.opt_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    *a.req_field() = "hello";
    *b.req_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *a.req_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.req_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    a.req_field() = "hello";
    b.req_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.req_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.req_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
}

// current equal-to codegen implements def/opt/req ref fields with a single
// code path, so no need to test the different cases separately
TEST_F(StructTest, equal_to_refs) {
  std::equal_to<BasicRefs> op;

  {
    BasicRefs a;
    BasicRefs b;

    a.def_field_ref() = nullptr;
    b.def_field_ref() = nullptr;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field_ref() = std::make_unique<HasInt>();
    *a.def_field_ref()->field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field_ref() = std::make_unique<HasInt>();
    *b.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field_ref()->field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    BasicRefs a;
    BasicRefs b;

    a.def_field_ref() = nullptr;
    b.def_field_ref() = nullptr;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field_ref() = std::make_unique<HasInt>();
    *a.def_field_ref()->field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field_ref() = std::make_unique<HasInt>();
    *b.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field_ref()->field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
}

TEST_F(StructTest, equal_to_refs_shared) {
  std::equal_to<BasicRefsShared> op;

  {
    BasicRefsShared a;
    BasicRefsShared b;

    a.def_field_ref() = nullptr;
    b.def_field_ref() = nullptr;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field_ref() = std::make_shared<HasInt>();
    *a.def_field_ref()->field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field_ref() = std::make_shared<HasInt>();
    *b.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field_ref()->field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    b.def_field_ref() = a.def_field_ref();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
}

TEST_F(StructTest, less) {
  std::less<Basic> op;

  {
    Basic a;
    Basic b;

    *b.def_field() = 3;
    *a.def_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field().ensure();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field().ensure();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.def_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.opt_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.opt_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    *b.req_field() = 3;
    *a.req_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.req_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.req_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    b.req_field() = 3;
    a.req_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.req_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.req_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
}

// currently, binary fields are handled specially, so they get their own tests
TEST_F(StructTest, less_binary) {
  std::less<BasicBinaries> op;

  {
    BasicBinaries a;
    BasicBinaries b;

    *b.def_field() = "hello";
    *a.def_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field().ensure();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field().ensure();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.def_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.opt_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.opt_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    *b.req_field() = "hello";
    *a.req_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.req_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.req_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    b.req_field() = "hello";
    a.req_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.req_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.req_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
}

// current less codegen implements def/opt/req ref fields with a single
// code path, so no need to test the different cases separately
TEST_F(StructTest, less_refs) {
  std::less<BasicRefs> op;

  {
    BasicRefs a;
    BasicRefs b;

    b.def_field_ref() = nullptr;
    a.def_field_ref() = nullptr;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field_ref() = std::make_unique<HasInt>();
    *b.def_field_ref()->field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field_ref() = std::make_unique<HasInt>();
    *a.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *b.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
}

TEST_F(StructTest, less_refs_shared) {
  std::less<BasicRefsShared> op;

  {
    BasicRefsShared a;
    BasicRefsShared b;

    b.def_field_ref() = nullptr;
    a.def_field_ref() = nullptr;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field_ref() = std::make_unique<HasInt>();
    *b.def_field_ref()->field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field_ref() = std::make_unique<HasInt>();
    *a.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *b.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field_ref() = a.def_field_ref();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    BasicRefsShared a;
    BasicRefsShared b;

    b.def_field_ref() = nullptr;
    a.def_field_ref() = nullptr;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field_ref() = std::make_unique<HasInt>();
    *b.def_field_ref()->field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field_ref() = std::make_unique<HasInt>();
    *a.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *b.def_field_ref()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field_ref() = a.def_field_ref();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
}

TEST_F(StructTest, custom_indirection) {
  IOBufIndirection a;
  a.foo()->raw = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "test");
  a.bar()->raw = "test2";
  IOBufIndirection b = a;
  EXPECT_EQ(a, b);
}

TEST_F(StructTest, small_sorted_vector) {
  using Set = SmallSortedVectorSet<int32_t>;
  using Map = SmallSortedVectorMap<int32_t, int32_t>;
  using serializer = apache::thrift::BinarySerializer;
  using Type = HasSmallSortedVector;
  EXPECT_TRUE(
      (std::is_same<folly::remove_cvref_t<decltype(*Type().set_field())>, Set>::
           value));
  EXPECT_TRUE(
      (std::is_same<folly::remove_cvref_t<decltype(*Type().map_field())>, Map>::
           value));

  Type o;
  o.set_field()->insert({1, 3, 5});
  o.map_field()->insert({{1, 4}, {3, 12}, {5, 20}});
  auto a = serializer::deserialize<HasSmallSortedVector>(
      serializer::serialize<std::string>(o));
  EXPECT_EQ(o, a);
  EXPECT_EQ(*o.set_field(), *a.set_field());
  EXPECT_EQ(*o.map_field(), *a.map_field());
}

TEST_F(StructTest, noexcept_move_annotation) {
  EXPECT_TRUE(std::is_nothrow_move_constructible<NoexceptMoveStruct>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<NoexceptMoveStruct>::value);
  NoexceptMoveStruct a;
  a.string_field() = "hello world";
  NoexceptMoveStruct b(std::move(a));
  EXPECT_EQ(b.string_field().value(), "hello world");
  NoexceptMoveStruct c;
  c = std::move(b);
  EXPECT_EQ(c.string_field().value(), "hello world");
}

TEST_F(StructTest, clear) {
  Basic obj;
  obj.def_field() = 7;
  obj.req_field() = 8;
  obj.opt_field() = 9;
  apache::thrift::clear(obj);
  EXPECT_FALSE(obj.def_field().is_set());
  EXPECT_EQ(0, *obj.def_field());
  EXPECT_EQ(0, *obj.req_field());
  EXPECT_FALSE(obj.opt_field());
}

TEST_F(StructTest, BasicIndirection) {
  BasicIndirection obj;
  obj.raw.def_field() = 7;
  obj.raw.req_field() = 8;
  obj.raw.opt_field() = 9;
  apache::thrift::CompactSerializer ser;
  auto obj2 =
      ser.deserialize<BasicIndirection>(ser.serialize<std::string>(obj));
  EXPECT_EQ(obj.raw, obj2.raw);
}

TEST_F(StructTest, EmptiableOptionalFieldsStruct) {
  EmptiableOptionalFieldsStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.int_field() = 1;
  EXPECT_FALSE(apache::thrift::empty(obj));

  apache::thrift::clear(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST_F(StructTest, NotEmptiableStruct) {
  NotEmptiableStruct obj;
  EXPECT_FALSE(apache::thrift::empty(obj));

  NotEmptiableTerseFieldsStruct obj2;
  EXPECT_FALSE(apache::thrift::empty(obj2));
}

TEST_F(StructTest, EmptyTerseStruct) {
  EmptyTerseStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST_F(StructTest, EmptiableTerseFieldsStruct) {
  EmptiableTerseFieldsStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.bool_field() = true;
  EXPECT_FALSE(apache::thrift::empty(obj));

  apache::thrift::clear(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST_F(StructTest, OptionalFieldsStruct) {
  OptionalFieldsStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.boxed_field() = HasInt();
  EXPECT_FALSE(apache::thrift::empty(obj));

  apache::thrift::clear(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST_F(StructTest, OptionalFieldsTerseStruct) {
  OptionalFieldsTerseStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.boxed_field() = HasInt();
  EXPECT_FALSE(apache::thrift::empty(obj));

  apache::thrift::clear(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}
} // namespace
