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
include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/field_mask.thrift"
include "thrift/lib/thrift/any.thrift"

cpp_include "thrift/test/AdapterTest.h"
cpp_include "thrift/lib/cpp2/protocol/FieldMask.h"
cpp_include "thrift/lib/cpp2/type/Any.h"
cpp_include "<unordered_map>"

package "apache.org/thrift/test"

struct Empty {}

struct Foo {
  1: i32 field1;
  3: i32 field2;
  @cpp.Type{template = "std::unordered_map"}
  11: map<string, Empty> field3;
}

struct Bar {
  // The field id is intentionally flipped to test if the methods use field id
  // and ordinal correctly.
  2: list<Foo> foos;
  1: Foo foo;
}

struct Baz {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  1: Foo foo;
}

union RecursiveUnion {
  1: Foo foo;
  2: Bar bar;
  3: Baz baz;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  4: RecursiveUnion recurse;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  5: map<string, RecursiveUnion> recurseMap;
}

struct HasMap {
  1: map<i64, Foo> foos;
  2: Foo foo;
  3: map<string, Foo> string_map;
}

struct Foo2 {
  // They have different types to use op::get_ with type tags.
  1: optional i32 field_1;
  2: optional i64 field_2;
}

struct Bar2 {
  1: optional Foo2 field_3;
  2: string field_4;
}

struct CustomDefault {
  1: string field = "default";
}

@thrift.Experimental
@thrift.TerseWrite
struct TerseWrite {
  1: i32 field;
  2: Foo foo;
}

struct SmartPointerStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional Foo2 unique;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional Foo2 shared;
  @thrift.Box
  3: optional Foo2 boxed;
}

struct SharedConstPointerStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: Foo2 unique;
  @cpp.Ref{type = cpp.RefType.Shared}
  2: optional Foo2 shared_const;
}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::protocol::MaskAdapter<Bar>"}
typedef field_mask.Mask TypedBarMask

struct MaskStruct {
  @cpp.Adapter{name = "::apache::thrift::protocol::MaskAdapter<Bar>"}
  1: field_mask.Mask mask;
  2: TypedBarMask mask2;
}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{
  name = "::apache::thrift::InlineAdapter<::apache::thrift::type::AnyData>",
}
typedef any.Any AdaptedAny

struct StructWithAny {
  1: any.Any rawAny;
  2: AdaptedAny adaptedAny;
  3: optional any.Any optAny;
}
