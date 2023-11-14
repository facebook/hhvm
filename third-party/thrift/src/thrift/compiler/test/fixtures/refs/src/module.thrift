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
include "thrift/annotation/java.thrift"
include "thrift/annotation/thrift.thrift"

namespace java.swift test.fixtures.refs

enum MyEnum {
  Zero = 0,
  One = 1,
}

union MyUnion {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: i32 anInteger;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: string aString;
}

union NonTriviallyDestructibleUnion {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: i32 int_field;
}

struct MyField {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional i64 opt_value;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: i64 value;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: required i64 req_value;

  @cpp.Ref{type = cpp.RefType.Unique}
  4: optional MyEnum opt_enum_value;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: MyEnum enum_value;
  @cpp.Ref{type = cpp.RefType.Unique}
  6: required MyEnum req_enum_value;

  @cpp.Ref{type = cpp.RefType.Unique}
  7: optional string opt_str_value;
  @cpp.Ref{type = cpp.RefType.Unique}
  8: string str_value;
  @cpp.Ref{type = cpp.RefType.Unique}
  9: required string req_str_value;
}

struct MyStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional MyField opt_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: MyField ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: required MyField req_ref;
}

struct StructWithUnion {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: MyUnion u;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: double aDouble;
  3: MyField f;
}

struct RecursiveStruct {
  @java.Recursive
  1: optional list<RecursiveStruct> mes;
}

struct StructWithContainers {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: list<i32> list_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: set<i32> set_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: map<i32, i32> map_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  4: list<i32> list_ref_unique;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  5: set<i32> set_ref_shared;
  @cpp.Ref{type = cpp.RefType.Shared}
  6: list<i32> list_ref_shared_const;
}

struct StructWithSharedConst {
  @cpp.Ref{type = cpp.RefType.Shared}
  1: optional MyField opt_shared_const;
  @cpp.Ref{type = cpp.RefType.Shared}
  2: MyField shared_const;
  @cpp.Ref{type = cpp.RefType.Shared}
  3: required MyField req_shared_const;
}

@cpp.EnumType{type = cpp.EnumUnderlyingType.I16}
enum TypedEnum {
  VAL1 = 0,
  VAL2 = 1,
}

struct Empty {}

struct StructWithRef {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: Empty def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional Empty opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: required Empty req_field;
}

struct StructWithBox {
  @thrift.Box
  1: optional string a;
  @thrift.Box
  2: optional list<i64> b;
  @thrift.Box
  3: optional StructWithRef c;
}

struct StructWithInternBox {
  @thrift.InternBox
  1: Empty field1;
  @thrift.InternBox
  2: MyField field2;
}

@thrift.Experimental
struct StructWithTerseInternBox {
  @thrift.InternBox
  @thrift.TerseWrite
  1: Empty field1;
  @thrift.InternBox
  @thrift.TerseWrite
  2: MyField field2;
}

struct AdaptedStructWithInternBox {
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.InternBox
  1: Empty field1;
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.InternBox
  2: MyField field2;
}

@thrift.Experimental
struct AdaptedStructWithTerseInternBox {
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.InternBox
  @thrift.TerseWrite
  1: Empty field1;
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.InternBox
  @thrift.TerseWrite
  2: MyField field2;
}

const StructWithRef kStructWithRef = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeUnique {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: Empty def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional Empty opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: required Empty req_field;
}

const StructWithRefTypeUnique kStructWithRefTypeUnique = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeShared {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: Empty def_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional Empty opt_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: required Empty req_field;
}

const StructWithRefTypeShared kStructWithRefTypeShared = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeSharedConst {
  @cpp.Ref{type = cpp.RefType.Shared}
  1: Empty def_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  2: optional Empty opt_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  3: required Empty req_field;
}

const StructWithRefTypeSharedConst kStructWithRefTypeSharedConst = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefAndAnnotCppNoexceptMoveCtor {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: Empty def_field;
}

struct StructWithString {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: string def_unique_string_ref = "...";
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: string def_shared_string_ref = "...";
  @cpp.Ref{type = cpp.RefType.Shared}
  3: string def_shared_string_const_ref = "...";
  @cpp.Ref{type = cpp.RefType.Unique}
  4: string unique_string_ref;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  5: string shared_string_ref;
}
