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

namespace java.swift test.fixtures.refs

enum MyEnum {
  Zero = 0,
  One = 1,
}

union MyUnion {
  1: i32 anInteger (cpp.ref = "true", cpp2.ref = "true");
  2: string aString (cpp.ref = "true", cpp2.ref = "true");
}

struct MyField {
  1: optional i64 opt_value (cpp.ref = "true", cpp2.ref = "true");
  2: i64 value (cpp.ref = "true", cpp2.ref = "true");
  3: required i64 req_value (cpp.ref = "true", cpp2.ref = "true");

  4: optional MyEnum opt_enum_value (cpp.ref = "true", cpp2.ref = "true");
  5: MyEnum enum_value (cpp.ref = "true", cpp2.ref = "true");
  6: required MyEnum req_enum_value (cpp.ref = "true", cpp2.ref = "true");

  7: optional string opt_str_value (cpp.ref = "true", cpp2.ref = "true");
  8: string str_value (cpp.ref = "true", cpp2.ref = "true");
  9: required string req_str_value (cpp.ref = "true", cpp2.ref = "true");
}

struct MyStruct {
  1: optional MyField opt_ref (cpp.ref = "true", cpp2.ref = "true");
  2: MyField ref (cpp.ref = "true", cpp2.ref = "true");
  3: required MyField req_ref (cpp.ref = "true", cpp2.ref = "true");
}

struct StructWithUnion {
  1: MyUnion u (cpp.ref = "true");
  2: double aDouble (cpp.ref = "true", cpp2.ref = "true");
  3: MyField f;
}

struct RecursiveStruct {
  1: optional list<RecursiveStruct> mes (swift.recursive_reference = "true");
}

struct StructWithContainers {
  1: list<i32> list_ref (cpp.ref = "true", cpp2.ref = "true");
  2: set<i32> set_ref (cpp.ref = "true", cpp2.ref = "true");
  3: map<i32, i32> map_ref (cpp.ref = "true", cpp2.ref = "true");
  4: list<i32> list_ref_unique (
    cpp.ref_type = "unique",
    cpp2.ref_type = "unique",
  );
  5: set<i32> set_ref_shared (
    cpp.ref_type = "shared",
    cpp2.ref_type = "shared",
  );
  6: list<i32> list_ref_shared_const (
    cpp.ref_type = "shared_const",
    cpp2.ref_type = "shared_const",
  );
}

struct StructWithSharedConst {
  1: optional MyField opt_shared_const (
    cpp.ref_type = "shared_const",
    cpp2.ref_type = "shared_const",
  );
  2: MyField shared_const (
    cpp.ref_type = "shared_const",
    cpp2.ref_type = "shared_const",
  );
  3: required MyField req_shared_const (
    cpp.ref_type = "shared_const",
    cpp2.ref_type = "shared_const",
  );
}

@cpp.EnumType{type = cpp.EnumUnderlyingType.I16}
enum TypedEnum {
  VAL1 = 0,
  VAL2 = 1,
}

struct Empty {}

struct StructWithRef {
  1: Empty def_field (cpp.ref);
  2: optional Empty opt_field (cpp.ref);
  3: required Empty req_field (cpp.ref);
}

struct StructWithBox {
  1: optional string a (thrift.box);
  2: optional list<i64> b (thrift.box);
  3: optional StructWithRef c (thrift.box);
}

@thrift.Experimental
struct StructWithInternBox {
  @thrift.InternBox
  1: Empty field1;
  @thrift.InternBox
  2: MyField field2;
  @thrift.InternBox
  @thrift.TerseWrite
  3: Empty field3;
  @thrift.InternBox
  @thrift.TerseWrite
  4: MyField field4;
}

@thrift.Experimental
struct AdaptedStructWithInternBox {
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.InternBox
  1: Empty field1;
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.InternBox
  2: MyField field2;
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.InternBox
  @thrift.TerseWrite
  3: Empty field3;
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.InternBox
  @thrift.TerseWrite
  4: MyField field4;
}

const StructWithRef kStructWithRef = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeUnique {
  1: Empty def_field (cpp.ref_type = "unique");
  2: optional Empty opt_field (cpp.ref_type = "unique");
  3: required Empty req_field (cpp.ref_type = "unique");
}

const StructWithRefTypeUnique kStructWithRefTypeUnique = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeShared {
  1: Empty def_field (cpp.ref_type = "shared");
  2: optional Empty opt_field (cpp.ref_type = "shared");
  3: required Empty req_field (cpp.ref_type = "shared");
}

const StructWithRefTypeShared kStructWithRefTypeShared = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeSharedConst {
  1: Empty def_field (cpp.ref_type = "shared_const");
  2: optional Empty opt_field (cpp.ref_type = "shared_const");
  3: required Empty req_field (cpp.ref_type = "shared_const");
}

const StructWithRefTypeSharedConst kStructWithRefTypeSharedConst = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefAndAnnotCppNoexceptMoveCtor {
  1: Empty def_field (cpp.ref);
}

struct StructWithString {
  1: string def_unique_string_ref = "..." (
    cpp.ref_type = "unique",
    cpp2.ref_type = "unique",
  );
  2: string def_shared_string_ref = "..." (
    cpp.ref_type = "shared",
    cpp2.ref_type = "shared",
  );
  3: string def_shared_string_const_ref = "..." (
    cpp.ref_type = "shared_const",
    cpp2.ref_type = "shared_const",
  );
  4: string unique_string_ref (
    cpp.ref_type = "unique",
    cpp2.ref_type = "unique",
  );
  5: string shared_string_ref (
    cpp.ref_type = "shared",
    cpp2.ref_type = "shared",
  );
}
