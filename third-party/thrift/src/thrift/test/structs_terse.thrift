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

include "thrift/annotation/thrift.thrift"
include "thrift/test/structs.thrift"
include "thrift/annotation/cpp.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 apache.thrift.test

struct EmptyTerseStruct {}

struct EmptiableTerseFieldsStruct {
  @cpp.DeprecatedTerseWrite
  1: bool bool_field;
  @cpp.DeprecatedTerseWrite
  2: byte byte_field;
  @cpp.DeprecatedTerseWrite
  3: i16 short_field;
  @cpp.DeprecatedTerseWrite
  4: i32 int_field;
  @cpp.DeprecatedTerseWrite
  5: i64 long_field;
  @cpp.DeprecatedTerseWrite
  6: float float_field;
  @cpp.DeprecatedTerseWrite
  7: double double_field;
  @cpp.DeprecatedTerseWrite
  8: binary binary_field;
  @cpp.DeprecatedTerseWrite
  9: string string_field;
  @cpp.DeprecatedTerseWrite
  10: list<i32> list_field;
  @cpp.DeprecatedTerseWrite
  11: set<i32> set_field;
  @cpp.DeprecatedTerseWrite
  12: map<i32, i32> map_field;
}

struct NotEmptiableTerseFieldsStruct {
  @cpp.DeprecatedTerseWrite
  1: bool bool_field;
  @cpp.DeprecatedTerseWrite
  2: byte byte_field;
  @cpp.DeprecatedTerseWrite
  3: i16 short_field;
  @cpp.DeprecatedTerseWrite
  4: i32 int_field;
  @cpp.DeprecatedTerseWrite
  5: i64 long_field;
  @cpp.DeprecatedTerseWrite
  6: float float_field;
  @cpp.DeprecatedTerseWrite
  7: double double_field;
  @cpp.DeprecatedTerseWrite
  8: binary binary_field;
  @cpp.DeprecatedTerseWrite
  9: string string_field;
  @cpp.DeprecatedTerseWrite
  10: list<i32> list_field;
  @cpp.DeprecatedTerseWrite
  11: set<i32> set_field;
  @cpp.DeprecatedTerseWrite
  12: map<i32, i32> map_field;
  13: EmptyTerseStruct struct_field;
}

struct BasicRefsSharedTerseWrites {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional structs.HasInt def_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional structs.HasInt shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: optional list<structs.HasInt> shared_fields;
  @cpp.Ref{type = cpp.RefType.Shared}
  4: optional structs.HasInt shared_field_const;
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional list<structs.HasInt> shared_fields_const;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  6: structs.HasInt shared_field_req;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  @cpp.DeprecatedTerseWrite
  7: list<structs.HasInt> shared_fields_req;
}

struct OptionalFieldsTerseStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional structs.HasInt def_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional structs.HasInt shared_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: optional list<structs.HasInt> shared_fields;
  @cpp.Ref{type = cpp.RefType.Shared}
  4: optional structs.HasInt shared_field_const;
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional list<structs.HasInt> shared_fields_const;
  @thrift.Box
  6: optional structs.HasInt boxed_field;
}

struct RefsWithStringAndContainerTerseWrites {
  @cpp.DeprecatedTerseWrite
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: list<string> string_list_field;
  @cpp.DeprecatedTerseWrite
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  2: string string_field;
}

struct NestedStruct {
  @cpp.DeprecatedTerseWrite
  1: i32 int_field = 42;
}

union NestedUnion {
  1: i32 int_field = 42;
}

exception NestedException {
  @cpp.DeprecatedTerseWrite
  1: i32 int_field = 42;
}

struct TerseFieldsWithCustomDefault {
  @cpp.DeprecatedTerseWrite
  1: bool bool_field = true;
  @cpp.DeprecatedTerseWrite
  2: byte byte_field = 10;
  @cpp.DeprecatedTerseWrite
  3: i16 short_field = 20;
  @cpp.DeprecatedTerseWrite
  4: i32 int_field = 30;
  @cpp.DeprecatedTerseWrite
  5: i64 long_field = 40;
  @cpp.DeprecatedTerseWrite
  6: float float_field = 50;
  @cpp.DeprecatedTerseWrite
  7: double double_field = 60;
  @cpp.DeprecatedTerseWrite
  8: binary binary_field = "70";
  @cpp.DeprecatedTerseWrite
  9: string string_field = "80";
  @cpp.DeprecatedTerseWrite
  10: list<i32> list_field = [90];
  @cpp.DeprecatedTerseWrite
  11: set<i32> set_field = [100];
  @cpp.DeprecatedTerseWrite
  12: map<i32, i32> map_field = {110: 10};

  13: NestedStruct struct_field;
  14: NestedUnion union_field;
  15: NestedException exception_field;

  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  @cpp.DeprecatedTerseWrite
  @cpp.AllowLegacyDeprecatedTerseWritesRef
  16: NestedStruct cpp_ref_struct_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  @cpp.DeprecatedTerseWrite
  @cpp.AllowLegacyDeprecatedTerseWritesRef
  17: NestedUnion cpp_ref_union_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  @cpp.DeprecatedTerseWrite
  @cpp.AllowLegacyDeprecatedTerseWritesRef
  18: NestedException cpp_ref_exception_field;

  @cpp.DeprecatedTerseWrite
  19: i32 redundant_custom_default = 0;

  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  20: NestedStruct cpp_shared_ref_struct_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  21: NestedUnion cpp_shared_ref_union_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  22: NestedException cpp_shared_ref_exception_field;
}
