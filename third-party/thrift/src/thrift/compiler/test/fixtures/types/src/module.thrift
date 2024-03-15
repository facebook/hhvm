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

include "included.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

package "apache.org/thrift/fixtures/types"

typedef binary TBinary

struct empty_struct {}

struct decorated_struct {
  1: string field;
} (cpp.declare_hash, cpp.declare_equal_to)

struct ContainerStruct {
  12: list<i32> fieldA;
  @cpp.Type{template = "std::list"}
  2: list<i32> fieldB;
  @cpp.Type{template = "std::deque"}
  3: list<i32> fieldC;
  @cpp.Type{template = "folly::fbvector"}
  4: list<i32> fieldD;
  @cpp.Type{template = "folly::small_vector"}
  5: list<i32> fieldE;
  6: set_i32_7194 fieldF;
  @cpp.Type{template = "folly::sorted_vector_map"}
  7: map_i32_string_1261 fieldG;
  8: included.SomeMap fieldH;
}

struct CppTypeStruct {
  @cpp.Type{name = "std::list<int32_t>"}
  1: list<i32> fieldA;
}

enum has_bitwise_ops {
  none = 0,
  zero = 1,
  one = 2,
  two = 4,
  three = 8,
} (cpp.declare_bitwise_ops)

enum is_unscoped {
  hello = 0,
  world = 1,
} (cpp.deprecated_enum_unscoped)

service SomeService {
  included.SomeMap bounce_map(1: included.SomeMap m);
  map<TBinary, i64> binary_keyed_map(1: list<i64> r);
}

struct VirtualStruct {
  1: i64 MyIntField;
} (cpp.virtual)

struct MyStructWithForwardRefEnum {
  1: MyForwardRefEnum a = NONZERO;
  2: MyForwardRefEnum b = MyForwardRefEnum.NONZERO;
}

enum MyForwardRefEnum {
  ZERO = 0,
  NONZERO = 12,
}

struct TrivialNumeric {
  1: i32 a;
  2: bool b;
}

struct TrivialNestedWithDefault {
  1: i32 z = 4;
  2: TrivialNumeric n = {'a': 3, 'b': true};
}

struct ComplexString {
  1: string a;
  2: map<string, i32> b;
}

struct ComplexNestedWithDefault {
  1: string z = '4';
  2: ComplexString n = {'a': '3', 'b': {'a': 3}};
}

@cpp.MinimizePadding
struct MinPadding {
  1: required byte small;
  2: required i64 big;
  3: required i16 medium;
  4: required i32 biggish;
  5: required byte tiny;
}

@thrift.Experimental
@cpp.MinimizePadding
@thrift.TerseWrite
struct MinPaddingWithCustomType {
  1: byte small;
  2: i64 big;
  @cpp.Adapter{name = "::my::Adapter"}
  3: i16 medium;
  4: i32 biggish;
  5: byte tiny;
}

struct MyStruct {
  1: i64 MyIntField;
  2: string MyStringField;
  3: i64 majorVer;
  4: MyDataItem data;
} (cpp.noncomparable)

struct MyDataItem {} (cpp.noncomparable)

struct Renaming {
  1: i64 foo (cpp.name = 'bar');
} (cpp.name = "Renamed")

struct AnnotatedTypes {
  1: TBinary_8623 binary_field;
  2: SomeListOfTypeMap_2468 list_field;
}

# Validates that C++ codegen performes appropriate topological sorting of
# structs definitions in the generated code
struct ForwardUsageRoot {
  # use the type before it is defined
  1: optional ForwardUsageStruct ForwardUsageStruct;
  # use the type before it is defined, but mark it as a ref in C++
  # (no need for it to be defined before this struct in generated code)
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional ForwardUsageByRef ForwardUsageByRef;
}

struct ForwardUsageStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional ForwardUsageRoot foo;
}

struct ForwardUsageByRef {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional ForwardUsageRoot foo;
}

struct IncompleteMap {
  1: optional map<i32, IncompleteMapDep> field;
}
struct IncompleteMapDep {}

struct CompleteMap {
  @cpp.Type{template = "std::unordered_map"}
  1: optional map<i32, CompleteMapDep> field;
}
struct CompleteMapDep {}

struct IncompleteList {
  @cpp.Type{template = "::std::list"}
  1: optional list<IncompleteListDep> field;
}
struct IncompleteListDep {}

struct CompleteList {
  @cpp.Type{template = "folly::small_vector"}
  1: optional list<CompleteListDep> field;
}
struct CompleteListDep {}

struct AdaptedList {
  1: optional list<AdaptedListDep> field;
}
@cpp.Adapter{
  name = "IdentityAdapter<detail::AdaptedListDep>",
  adaptedType = "detail::AdaptedListDep",
}
struct AdaptedListDep {
  1: AdaptedList field;
}

struct DependentAdaptedList {
  1: optional list<DependentAdaptedListDep> field;
}
@cpp.Adapter{name = "IdentityAdapter<detail::DependentAdaptedListDep>"}
struct DependentAdaptedListDep {
  @thrift.Box
  1: optional i16 field;
}

# Allocator-aware struct with allocator-aware fields
struct AllocatorAware {
  1: list_i32_9187 aa_list;
  2: set_i32_7070 aa_set;
  3: map_i32_i32_9565 aa_map;
  4: string_5252 aa_string;
  5: i32 not_a_container;
  @cpp.Ref{type = cpp.RefType.Unique}
  6: i32_9314 aa_unique;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  7: i32_9314 aa_shared;
} (cpp.allocator = "some_allocator")

# Allocator-aware struct with no allocator-aware fields
struct AllocatorAware2 {
  1: i32 not_a_container;
  @thrift.Box
  2: optional i32 box_field;
} (cpp.allocator = "some_allocator")

typedef i32 IntTypedef
@cpp.Type{name = "std::uint32_t"}
typedef IntTypedef UintTypedef

struct TypedefStruct {
  1: i32 i32_field;
  2: IntTypedef IntTypedef_field;
  3: UintTypedef UintTypedef_field;
}

struct StructWithDoubleUnderscores {
  1: i32 __field;
}

// The following were automatically generated and may benefit from renaming.
typedef included.SomeListOfTypeMap (
  noop_annotation = "1",
) SomeListOfTypeMap_2468
typedef TBinary (noop_annotation = "1") TBinary_8623
typedef i32 (cpp.use_allocator = "1") i32_9314
typedef list<i32> (cpp.use_allocator = "1") list_i32_9187
typedef map<i32, i32> (cpp.use_allocator = "1") map_i32_i32_9565
typedef map<i32, string> (
  rust.type = "sorted_vector_map::SortedVectorMap",
) map_i32_string_1261
typedef set<i32> (cpp.use_allocator = "1") set_i32_7070
@cpp.Type{template = "folly::sorted_vector_set"}
typedef set<i32> (rust.type = "sorted_vector_map::SortedVectorSet") set_i32_7194
typedef string (cpp.use_allocator = "1") string_5252
