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
include "thrift/test/adapter.thrift"
cpp_include "thrift/test/AdapterTest.h"

namespace cpp2 cpp2

struct RecursiveStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: RecursiveStruct def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional RecursiveStruct opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required RecursiveStruct req_field;
}

struct PlainStruct {
  1: i32 field;
}

@thrift.Experimental
struct EmptiableStruct {
  @thrift.TerseWrite
  1: i32 field;
}

struct ReferringStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: PlainStruct def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional PlainStruct opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required PlainStruct req_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  4: PlainStruct def_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: optional PlainStruct opt_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  6: required PlainStruct req_unique_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  7: PlainStruct def_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  8: optional PlainStruct opt_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  9: required PlainStruct req_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  10: PlainStruct def_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  11: optional PlainStruct opt_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  12: required PlainStruct req_shared_const_field;
  @thrift.Box
  13: optional PlainStruct opt_box_field;
}

struct ReferringStructWithBaseTypeFields {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: i64 def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional i64 opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required i64 req_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  4: i64 def_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: optional i64 opt_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  6: required i64 req_unique_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  7: i64 def_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  8: optional i64 opt_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  9: required i64 req_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  10: i64 def_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  11: optional i64 opt_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  12: required i64 req_shared_const_field;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.box": "1"}}
  13: optional i64 opt_box_field;
}

struct ReferringStructWithStringFields {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: string def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional string opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required string req_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  4: string def_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: optional string opt_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  6: required string req_unique_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  7: string def_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  8: optional string opt_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  9: required string req_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  10: string def_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  11: optional string opt_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  12: required string req_shared_const_field;
  @thrift.Box
  13: optional string opt_box_field;
}

struct ReferringStructWithListFields {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: list<i32> def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional list<i32> opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required list<i32> req_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  4: list<i32> def_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: optional list<i32> opt_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  6: required list<i32> req_unique_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  7: list<i32> def_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  8: optional list<i32> opt_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  9: required list<i32> req_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  10: list<i32> def_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  11: optional list<i32> opt_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  12: required list<i32> req_shared_const_field;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.box": "1"}}
  13: optional list<i32> opt_box_field;
}

struct ReferringStructWithSetFields {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: set<i32> def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional set<i32> opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required set<i32> req_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  4: set<i32> def_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: optional set<i32> opt_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  6: required set<i32> req_unique_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  7: set<i32> def_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  8: optional set<i32> opt_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  9: required set<i32> req_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  10: set<i32> def_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  11: optional set<i32> opt_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  12: required set<i32> req_shared_const_field;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.box": "1"}}
  13: optional set<i32> opt_box_field;
}

struct ReferringStructWithMapFields {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: map<i32, i32> def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional map<i32, i32> opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required map<i32, i32> req_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  4: map<i32, i32> def_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: optional map<i32, i32> opt_unique_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  6: required map<i32, i32> req_unique_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  7: map<i32, i32> def_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  8: optional map<i32, i32> opt_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  9: required map<i32, i32> req_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  10: map<i32, i32> def_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  11: optional map<i32, i32> opt_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  12: required map<i32, i32> req_shared_const_field;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.box": "1"}}
  13: optional map<i32, i32> opt_box_field;
}

struct TypeAdapterRefStruct {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: string def_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional string opt_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  3: required string req_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  4: string def_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional string opt_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  6: required string req_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @thrift.Box
  7: optional string opt_box_field;
}

struct FieldAdapterRefStruct {
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: string def_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional string opt_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  3: required string req_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  4: string def_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional string opt_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  6: required string req_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @thrift.Box
  7: optional string opt_box_field;
  8: string meta;
}

struct StructAdapterRefStruct {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: adapter.DirectlyAdaptedStruct def_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional adapter.DirectlyAdaptedStruct opt_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  3: required adapter.DirectlyAdaptedStruct req_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  4: adapter.DirectlyAdaptedStruct def_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional adapter.DirectlyAdaptedStruct opt_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  6: required adapter.DirectlyAdaptedStruct req_shared_const_field;
  @thrift.Box
  7: optional adapter.DirectlyAdaptedStruct opt_box_field;
}

struct DoubleAdaptedRefStruct {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: adapter.DirectlyAdaptedStruct def_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional adapter.DirectlyAdaptedStruct opt_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  3: required adapter.DirectlyAdaptedStruct req_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  4: adapter.DirectlyAdaptedStruct def_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional adapter.DirectlyAdaptedStruct opt_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  6: required adapter.DirectlyAdaptedStruct req_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @thrift.Box
  7: optional adapter.DirectlyAdaptedStruct opt_box_field;
}

struct StructuredAnnotation {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional PlainStruct opt_unique_field;

  @cpp.Ref{type = cpp.RefType.Shared}
  2: optional PlainStruct opt_shared_field;

  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: optional PlainStruct opt_shared_mutable_field;

  @thrift.InternBox
  4: PlainStruct intern_box_field;
}

@thrift.Experimental
struct TerseInternBox {
  @thrift.InternBox
  @thrift.TerseWrite
  1: EmptiableStruct intern_box_field;
}

struct StructWithString {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: string def_unique_string_ref = "...";
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  2: string def_shared_string_ref = "...";
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  3: string def_shared_string_const_ref = "...";
}

union ReferringUnionWithCppRef {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: string box_string;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  2: PlainStruct box_plain;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: ReferringUnionWithCppRef box_self;
}

union ReferringUnion {
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.box": "1"}}
  1: string box_string;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.box": "1"}}
  2: PlainStruct box_plain;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.box": "1"}}
  3: ReferringUnion box_self;
}

union NonTriviallyDestructibleUnion {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: i32 int_field;
}
