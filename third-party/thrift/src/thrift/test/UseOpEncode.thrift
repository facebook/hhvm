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

namespace cpp2 apache.thrift.test

include "thrift/annotation/cpp.thrift"
cpp_include "thrift/test/AdapterTest.h"

struct Foo {
  1: i32 field;
}

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef Foo AdaptedFoo

struct Bar {
  1: list<AdaptedFoo> list_field;
}

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef Bar AdaptedBar

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef i32 AdaptedI32

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef set<i32> AdaptedSetOfI32

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef set<AdaptedI32> AdaptedSetOfAdaptedI32

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef set<AdaptedSetOfAdaptedI32> AdaptedSetOfAdaptedSetOfAdaptedI32

enum Enum {
  first = 1,
  second = 2,
}

@cpp.UseOpEncode
struct OpEncodeStruct {
  1: i32 int_field;
  2: Enum enum_field;
  3: Foo foo_field;
  4: AdaptedFoo adapted_field;
  5: list<AdaptedFoo> list_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  6: optional list<AdaptedFoo> list_shared_ptr_field;
  7: list<AdaptedFoo> list_cpp_type_field;
  8: set<AdaptedFoo> set_field;
  9: map<AdaptedFoo, AdaptedFoo> map_field;
  10: map<i32, list<AdaptedFoo>> nested_field;
  11: Bar bar_field;
  12: AdaptedI32 adapted_int_field;
  13: list<AdaptedI32> list_int_field;

  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  14: list<AdaptedFoo> adapted_list_field;
  15: string meta;

  16: binary_8095 buf;

  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  17: list<AdaptedFoo> inplace_adapted_list_field;

  18: i64_6519 timestamp;
  19: map<AdaptedFoo, map<AdaptedBar, AdaptedI32>> nested_map_field;

  20: list<AdaptedSetOfI32> field20;
  21: list<AdaptedSetOfAdaptedI32> field21;
  22: list<AdaptedSetOfAdaptedSetOfAdaptedI32> field22;
}

struct Baz {
  1: i32 int_field;
  2: Enum enum_field;
  3: AdaptedFoo adapted_field;
  4: list<Foo> list_field;
  5: map<i32, Foo> map_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  6: optional list<Foo> list_shared_ptr_field;
}

@cpp.UseOpEncode
struct BazWithUseOpEncode {
  1: i32 int_field;
  2: Enum enum_field;
  3: AdaptedFoo adapted_field;
  4: list<AdaptedFoo> list_field;
  5: map<i32, AdaptedFoo> map_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  6: optional list<AdaptedFoo> list_shared_ptr_field;
}

// The following were automatically generated and may benefit from renaming.
@cpp.Type{name = "::apache::thrift::test::IndirectionIOBuf"}
typedef binary (cpp.indirection = "1") binary_8095
@cpp.Type{name = "Timestamp"}
typedef i64 (cpp.indirection = "1") i64_6519
