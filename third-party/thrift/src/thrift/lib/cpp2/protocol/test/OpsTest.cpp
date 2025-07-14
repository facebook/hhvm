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

#include <thrift/lib/cpp2/protocol/Ops.h>

#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/OpsTest_types.h>
#include <thrift/lib/cpp2/reflection/testing.h>
#include <thrift/lib/cpp2/type/Any.h>

#include <gtest/gtest.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace facebook::thrift::test;

TEST(OpsTest, filter) {
  Struct s;
  s.bool_field() = true;
  s.int_field() = 42;
  s.struct_field()->int_field() = 43;
  s.struct_field()->string_field() = "foo";
  NestedStruct i1;
  i1.int_field() = 1;
  i1.string_field() = "bar";
  NestedStruct i2;
  i2.int_field() = 2;
  i2.string_field() = "baz";
  s.int_map_field() = {{1, i1}, {2, i2}};
  s.string_map_field() = {{"foo", true}, {"bar", false}};
  s.any_field() = type::AnyData::toAny(*s.struct_field()).toThrift();

  auto wholeBuf = CompactSerializer::serialize<std::string>(s);

  auto filterWithCursor = [&](const auto& mask, const auto& src) {
    auto buf = folly::IOBuf::wrapBuffer(src.data(), src.size());
    return CompactSerializer::deserialize<Struct>(
        protocol::filterSerialized<CompactSerializer>(MaskRef{mask}, buf)
            .get());
  };

  auto staticFilter = [&](const auto& mask, const Struct& src) {
    return protocol::filter(mask, src);
  };

  // Test all fields included.
  MaskBuilder<Struct> mb(noneMask());
  mb.includes<ident::bool_field>();
  mb.includes<ident::int_field>();
  mb.includes<ident::struct_field>();
  mb.includes<ident::int_map_field>();
  mb.includes<ident::string_map_field>();
  mb.includes<ident::any_field>();
  auto wholeMask = mb.toThrift();

  EXPECT_THRIFT_EQ(s, staticFilter(wholeMask, s));
  EXPECT_THRIFT_EQ(s, filterWithCursor(wholeMask, wholeBuf));
  EXPECT_THRIFT_EQ(s, filterWithCursor(allMask(), wholeBuf));

  // Test all fields excluded.
  mb.reset_to_all();
  mb.excludes<ident::bool_field>();
  mb.excludes<ident::int_field>();
  mb.excludes<ident::struct_field>();
  mb.excludes<ident::int_map_field>();
  mb.excludes<ident::string_map_field>();
  mb.excludes<ident::any_field>();
  auto emptyMask = mb.toThrift();

  EXPECT_THRIFT_EQ(Struct{}, staticFilter(emptyMask, s));
  EXPECT_THRIFT_EQ(Struct{}, filterWithCursor(emptyMask, wholeBuf));
  EXPECT_THRIFT_EQ(Struct{}, filterWithCursor(noneMask(), wholeBuf));

  // Test partial inclusion.
  mb.reset_to_none();
  mb.includes<ident::struct_field>(MaskBuilder<NestedStruct>(noneMask())
                                       .includes<ident::int_field>()
                                       .toThrift());
  mb.includes_map_element<ident::int_map_field>(1);
  mb.includes_map_element<ident::string_map_field>("foo");
  mb.includes_type<ident::any_field>(type::struct_t<NestedStruct>{});
  auto partialMask = mb.toThrift();

  Struct filtered = filterWithCursor(partialMask, wholeBuf);
  EXPECT_EQ(*filtered.int_field(), 0);
  EXPECT_EQ(*filtered.struct_field()->int_field(), 43);
  EXPECT_EQ(*filtered.struct_field()->string_field(), "");
  EXPECT_EQ(filtered.int_map_field()->size(), 1);
  EXPECT_EQ(filtered.int_map_field()->at(1).int_field(), 1);
  EXPECT_EQ(filtered.string_map_field()->size(), 1);
  EXPECT_EQ(filtered.string_map_field()->at("foo"), true);
  EXPECT_THRIFT_EQ(*filtered.any_field(), *s.any_field());

  // Test partial exclusion.
  mb.reset_to_all();
  mb.excludes<ident::struct_field>(MaskBuilder<NestedStruct>(noneMask())
                                       .includes<ident::string_field>()
                                       .toThrift());
  mb.excludes_map_element<ident::int_map_field>(2);
  mb.excludes_map_element<ident::string_map_field>("bar");
  mb.excludes_type<ident::any_field>(type::struct_t<NestedStruct>{});
  auto partialMask2 = mb.toThrift();

  filtered = filterWithCursor(partialMask2, wholeBuf);
  EXPECT_EQ(*filtered.int_field(), 42);
  EXPECT_EQ(*filtered.struct_field()->int_field(), 43);
  EXPECT_EQ(*filtered.struct_field()->string_field(), "");
  EXPECT_EQ(filtered.int_map_field()->size(), 1);
  EXPECT_EQ(filtered.int_map_field()->at(1).int_field(), 1);
  EXPECT_EQ(filtered.string_map_field()->size(), 1);
  EXPECT_EQ(filtered.string_map_field()->at("foo"), true);
  EXPECT_THRIFT_EQ(*filtered.any_field(), type::AnyStruct{});

  // Test nested mask.
  mb.reset_to_none();
  mb.includes_map_element<ident::int_map_field>(
      1,
      MaskBuilder<NestedStruct>(noneMask())
          .includes<ident::string_field>()
          .toThrift());
  mb.includes_type<ident::any_field>(
      type::struct_t<NestedStruct>{},
      MaskBuilder<NestedStruct>(noneMask())
          .includes<ident::string_field>()
          .toThrift());
  auto nestedMask = mb.toThrift();

  filtered = filterWithCursor(nestedMask, wholeBuf);
  EXPECT_EQ(filtered.int_map_field()->size(), 1);
  auto nested = filtered.int_map_field()->at(1);
  EXPECT_EQ(nested.int_field(), 0);
  EXPECT_EQ(nested.string_field(), "bar");
  nested = type::AnyData(*filtered.any_field()).get<NestedStruct>();
  EXPECT_EQ(nested.int_field(), 0);
  EXPECT_EQ(nested.string_field(), "foo");
}
