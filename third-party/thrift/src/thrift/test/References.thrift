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
  1: RecursiveStruct def_field (cpp2.ref = "true");
  2: optional RecursiveStruct opt_field (cpp2.ref = "true");
  3: required RecursiveStruct req_field (cpp2.ref = "true");
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
  1: PlainStruct def_field (cpp2.ref = "true");
  2: optional PlainStruct opt_field (cpp2.ref = "true");
  3: required PlainStruct req_field (cpp2.ref = "true");
  4: PlainStruct def_unique_field (cpp2.ref_type = "unique");
  5: optional PlainStruct opt_unique_field (cpp2.ref_type = "unique");
  6: required PlainStruct req_unique_field (cpp2.ref_type = "unique");
  7: PlainStruct def_shared_field (cpp2.ref_type = "shared");
  8: optional PlainStruct opt_shared_field (cpp2.ref_type = "shared");
  9: required PlainStruct req_shared_field (cpp2.ref_type = "shared");
  10: PlainStruct def_shared_const_field (cpp2.ref_type = "shared_const");
  11: optional PlainStruct opt_shared_const_field (
    cpp2.ref_type = "shared_const",
  );
  12: required PlainStruct req_shared_const_field (
    cpp2.ref_type = "shared_const",
  );
  13: optional PlainStruct opt_box_field (thrift.box);
}

struct ReferringStructWithBaseTypeFields {
  1: i64 def_field (cpp2.ref = "true");
  2: optional i64 opt_field (cpp2.ref = "true");
  3: required i64 req_field (cpp2.ref = "true");
  4: i64 def_unique_field (cpp2.ref_type = "unique");
  5: optional i64 opt_unique_field (cpp2.ref_type = "unique");
  6: required i64 req_unique_field (cpp2.ref_type = "unique");
  7: i64 def_shared_field (cpp2.ref_type = "shared");
  8: optional i64 opt_shared_field (cpp2.ref_type = "shared");
  9: required i64 req_shared_field (cpp2.ref_type = "shared");
  10: i64 def_shared_const_field (cpp2.ref_type = "shared_const");
  11: optional i64 opt_shared_const_field (cpp2.ref_type = "shared_const");
  12: required i64 req_shared_const_field (cpp2.ref_type = "shared_const");
  13: optional i64 opt_box_field (cpp.box);
}

struct ReferringStructWithStringFields {
  1: string def_field (cpp2.ref = "true");
  2: optional string opt_field (cpp2.ref = "true");
  3: required string req_field (cpp2.ref = "true");
  4: string def_unique_field (cpp2.ref_type = "unique");
  5: optional string opt_unique_field (cpp2.ref_type = "unique");
  6: required string req_unique_field (cpp2.ref_type = "unique");
  7: string def_shared_field (cpp2.ref_type = "shared");
  8: optional string opt_shared_field (cpp2.ref_type = "shared");
  9: required string req_shared_field (cpp2.ref_type = "shared");
  10: string def_shared_const_field (cpp2.ref_type = "shared_const");
  11: optional string opt_shared_const_field (cpp2.ref_type = "shared_const");
  12: required string req_shared_const_field (cpp2.ref_type = "shared_const");
  13: optional string opt_box_field (thrift.box);
}

struct ReferringStructWithListFields {
  1: list<i32> def_field (cpp2.ref = "true");
  2: optional list<i32> opt_field (cpp2.ref = "true");
  3: required list<i32> req_field (cpp2.ref = "true");
  4: list<i32> def_unique_field (cpp2.ref_type = "unique");
  5: optional list<i32> opt_unique_field (cpp2.ref_type = "unique");
  6: required list<i32> req_unique_field (cpp2.ref_type = "unique");
  7: list<i32> def_shared_field (cpp2.ref_type = "shared");
  8: optional list<i32> opt_shared_field (cpp2.ref_type = "shared");
  9: required list<i32> req_shared_field (cpp2.ref_type = "shared");
  10: list<i32> def_shared_const_field (cpp2.ref_type = "shared_const");
  11: optional list<i32> opt_shared_const_field (
    cpp2.ref_type = "shared_const",
  );
  12: required list<i32> req_shared_const_field (
    cpp2.ref_type = "shared_const",
  );
  13: optional list<i32> opt_box_field (cpp.box);
}

struct ReferringStructWithSetFields {
  1: set<i32> def_field (cpp2.ref = "true");
  2: optional set<i32> opt_field (cpp2.ref = "true");
  3: required set<i32> req_field (cpp2.ref = "true");
  4: set<i32> def_unique_field (cpp2.ref_type = "unique");
  5: optional set<i32> opt_unique_field (cpp2.ref_type = "unique");
  6: required set<i32> req_unique_field (cpp2.ref_type = "unique");
  7: set<i32> def_shared_field (cpp2.ref_type = "shared");
  8: optional set<i32> opt_shared_field (cpp2.ref_type = "shared");
  9: required set<i32> req_shared_field (cpp2.ref_type = "shared");
  10: set<i32> def_shared_const_field (cpp2.ref_type = "shared_const");
  11: optional set<i32> opt_shared_const_field (cpp2.ref_type = "shared_const");
  12: required set<i32> req_shared_const_field (cpp2.ref_type = "shared_const");
  13: optional set<i32> opt_box_field (cpp.box);
}

struct ReferringStructWithMapFields {
  1: map<i32, i32> def_field (cpp2.ref = "true");
  2: optional map<i32, i32> opt_field (cpp2.ref = "true");
  3: required map<i32, i32> req_field (cpp2.ref = "true");
  4: map<i32, i32> def_unique_field (cpp2.ref_type = "unique");
  5: optional map<i32, i32> opt_unique_field (cpp2.ref_type = "unique");
  6: required map<i32, i32> req_unique_field (cpp2.ref_type = "unique");
  7: map<i32, i32> def_shared_field (cpp2.ref_type = "shared");
  8: optional map<i32, i32> opt_shared_field (cpp2.ref_type = "shared");
  9: required map<i32, i32> req_shared_field (cpp2.ref_type = "shared");
  10: map<i32, i32> def_shared_const_field (cpp2.ref_type = "shared_const");
  11: optional map<i32, i32> opt_shared_const_field (
    cpp2.ref_type = "shared_const",
  );
  12: required map<i32, i32> req_shared_const_field (
    cpp2.ref_type = "shared_const",
  );
  13: optional map<i32, i32> opt_box_field (cpp.box);
}

struct TypeAdapterRefStruct {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: string def_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional string opt_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: required string req_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  4: string def_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional string opt_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  6: required string req_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @thrift.Box
  7: optional string opt_box_field;
}

struct FieldAdapterRefStruct {
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: string def_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional string opt_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: required string req_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.Shared}
  4: string def_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional string opt_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @cpp.Ref{type = cpp.RefType.Shared}
  6: required string req_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  @thrift.Box
  7: optional string opt_box_field;
  8: string meta;
}

struct StructAdapterRefStruct {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: adapter.DirectlyAdaptedStruct def_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional adapter.DirectlyAdaptedStruct opt_shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: required adapter.DirectlyAdaptedStruct req_shared_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  4: adapter.DirectlyAdaptedStruct def_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional adapter.DirectlyAdaptedStruct opt_shared_const_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  6: required adapter.DirectlyAdaptedStruct req_shared_const_field;
  @thrift.Box
  7: optional adapter.DirectlyAdaptedStruct opt_box_field;
}

struct DoubleAdaptedRefStruct {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: adapter.DirectlyAdaptedStruct def_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional adapter.DirectlyAdaptedStruct opt_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: required adapter.DirectlyAdaptedStruct req_shared_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  4: adapter.DirectlyAdaptedStruct def_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional adapter.DirectlyAdaptedStruct opt_shared_const_field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @cpp.Ref{type = cpp.RefType.Shared}
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
  1: string def_unique_string_ref = "..." (cpp.ref_type = "unique");
  2: string def_shared_string_ref = "..." (cpp.ref_type = "shared");
  3: string def_shared_string_const_ref = "..." (cpp.ref_type = "shared_const");
}

union ReferringUnionWithCppRef {
  1: string box_string (cpp.ref);
  2: PlainStruct box_plain (cpp.ref);
  3: ReferringUnionWithCppRef box_self (cpp.ref);
}

union ReferringUnion {
  1: string box_string (cpp.box);
  2: PlainStruct box_plain (cpp.box);
  3: ReferringUnion box_self (cpp.box);
}

union NonTriviallyDestructibleUnion {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: i32 int_field;
}
