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

namespace cpp2 apache.thrift.test

struct EmptyTerseStruct {}

struct EmptiableTerseFieldsStruct {
  1: bool bool_field;
  2: byte byte_field;
  3: i16 short_field;
  4: i32 int_field;
  5: i64 long_field;
  6: float float_field;
  7: double double_field;
  8: binary binary_field;
  9: string string_field;
  10: list<i32> list_field;
  11: set<i32> set_field;
  12: map<i32, i32> map_field;
}

struct NotEmptiableTerseFieldsStruct {
  1: bool bool_field;
  2: byte byte_field;
  3: i16 short_field;
  4: i32 int_field;
  5: i64 long_field;
  6: float float_field;
  7: double double_field;
  8: binary binary_field;
  9: string string_field;
  10: list<i32> list_field;
  11: set<i32> set_field;
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
  6: structs.HasInt shared_field_req;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
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
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: list<string> string_list_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: string string_field;
}
