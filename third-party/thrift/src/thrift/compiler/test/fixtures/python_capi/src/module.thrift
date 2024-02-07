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

include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/patch.thrift"
include "thrift/compiler/test/fixtures/python_capi/src/thrift_dep.thrift"
include "thrift/compiler/test/fixtures/python_capi/src/serialized_dep.thrift"
include "thrift/lib/thrift/id.thrift"
include "thrift/lib/thrift/schema.thrift"

cpp_include "<deque>"
cpp_include "<unordered_map>"
cpp_include "<unordered_set>"
cpp_include "<folly/container/F14Set.h>"
cpp_include "<folly/FBString.h>"
cpp_include "<folly/container/F14Map.h>"
cpp_include "<folly/small_vector.h>"
cpp_include "thrift/test/python_capi/adapter.h"
cpp_include "thrift/lib/cpp2/util/ManagedStringView.h"

package "test.dev/fixtures/python_capi"

@cpp.Type{name = "uint64_t"}
typedef i64 uint64
typedef uint64 ui64

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
}

@cpp.Name{value = "NormalDecentEnum"}
enum AnnoyingEnum {
  @cpp.Name{value = "l0O1"}
  FOO = 1,
  @cpp.Name{value = "FuBaR"}
  BAR = 2,
}

@patch.GeneratePatch
struct MyStruct {
  1: i64 inty;
  2: string stringy;
  3: MyDataItem myItemy;
  4: MyEnum myEnumy;
  5: bool booly (cpp.name = "boulet");
  6: list<float> floatListy;
  7: map<binary, string> strMappy;
  8: set<i32> intSetty;
}

@patch.GeneratePatch
struct MyDataItem {
  1: string s;
}

@cpp.Adapter{name = "::thrift::test::lib::StructDoubler"}
@scope.Transitive
struct TransitiveDoubler {}

@TransitiveDoubler
struct DoubledPair {
  1: string s;
  2: i32 x;
}

struct StringPair {
  1: string normal;
  @cpp.Adapter{name = "::thrift::test::lib::StringDoubler"}
  2: string doubled;
}

@cpp.Name{value = "VapidStruct"}
struct EmptyStruct {}

typedef byte signed_byte
@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf
@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr

struct PrimitiveStruct {
  1: bool booly;
  2: signed_byte charry;
  @cpp.Type{name = "uint16_t"}
  3: i16 shorty (cpp.name = "shortay");
  5: i32 inty;
  @cpp.Type{name = "uint64_t"}
  7: i64 longy;
  8: optional float floaty;
  @thrift.Box
  9: optional double dubby;
  @cpp.Ref{type = cpp.RefType.Unique}
  12: optional string stringy;
  @cpp.Ref{type = cpp.RefType.Shared}
  13: optional binary bytey;
  14: IOBuf buffy;
  15: IOBufPtr pointbuffy;
  18: MyStruct patched_struct;
  19: EmptyStruct empty_struct;
  @cpp.Type{name = "folly::fbstring"}
  20: binary fbstring;
  @cpp.Type{name = "::apache::thrift::ManagedStringViewWithConversions"}
  21: string managed_string_view;
  22: thrift_dep.SomeError some_error;
}

struct AdaptedFields {
  1: id.ProtocolId adapted_int;
  2: list<id.FieldId> list_adapted_int;
  3: schema.AnnotationIds set_adapted_int;
  @cpp.Adapter{
    name = "::apache::thrift::type::detail::StrongIntegerAdapter<::apache::thrift::type::ValueId>",
  }
  4: id.ExternId inline_adapted_int;
}

struct ListStruct {
  1: list<bool> boolz;
  2: optional list<i64> intz;
  @thrift.Box
  3: optional list<string> stringz;
  @cpp.Type{template = "std::deque"}
  4: list<binary> encoded;
  @cpp.Type{name = "std::deque<uint64_t>"}
  5: list<i64> uidz;
  6: list<list<double>> matrix;
  @cpp.Type{name = "folly::small_vector<folly::small_vector<uint8_t>>"}
  7: list<list<byte>> ucharz;
  @cpp.Type{name = "folly::fbvector<folly::fbvector<folly::fbvector<uint8_t>>>"}
  8: list<list<list<signed_byte>>> voxels;
  9: list<IOBufPtr> buf_ptrs;
} (cpp.noncopyable)
typedef ListStruct ListAlias

struct SetStruct {
  1: set<MyEnum> enumz;
  2: optional set<i32> intz;
  @thrift.Box
  3: optional set<binary> binnaz;
  @cpp.Type{template = "std::unordered_set"}
  4: set<binary> encoded;
  @cpp.Type{name = "std::unordered_set<uint64_t>"}
  5: set<i64> uidz;
  @cpp.Type{name = "folly::F14FastSet<uint8_t>"}
  6: set<byte> charz;
  7: list<set<i64>> setz;
}

struct MapStruct {
  1: map<MyEnum, string> enumz;
  2: optional map<i32, string> intz;
  @thrift.Box
  3: optional map<binary, PrimitiveStruct> binnaz;
  @cpp.Type{template = "std::unordered_map"}
  4: map<string, double> encoded;
  @cpp.Type{name = "std::unordered_map<uint64_t, float>"}
  5: map<i64, float> flotz;
  6: list<map<i32, i64>> map_list;
  7: map<i32, list<i64>> list_map;
  @cpp.Type{name = "folly::F14FastMap<int, folly::fbvector<double>>"}
  8: map<i32, list<double>> fast_list_map;
  9: map<binary, IOBufPtr> buf_map;
  10: map<ui64, list<ui64>> unsigned_list_map;
}

struct ComposeStruct {
  1: MyEnum enum_;
  2: AnnoyingEnum renamed_;
  3: PrimitiveStruct primitive;
  @cpp.Ref{type = cpp.RefType.Shared}
  4: ListAlias aliased;
  6: thrift_dep.DepStruct xstruct;
  5: thrift_dep.DepEnum xenum;
  8: serialized_dep.SerializedStruct serial_struct;
  7: list<thrift_dep.DepStruct> friends;
  9: serialized_dep.SerializedUnion serial_union;
  10: serialized_dep.SerializedError serial_error;
} (cpp.noncopyable)

@cpp.Name{value = "Shallot"}
union Onion {
  1: MyEnum myEnum;
  2: PrimitiveStruct myStruct;
  6: set<i64> intSet;
  4: string myString;
  8: list<double> doubleList;
  9: map<binary, string> strMap;
  10: id.ProtocolId adaptedInt;
}
