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
include "thrift/annotation/python.thrift"

cpp_include "thrift/test/AdapterTest.h"

@thrift.Experimental
package "facebook.com/thrift/test"

namespace cpp2 apache.thrift.test.terse_write
namespace py3 apache.thrift.test.terse_write

enum MyEnum {
  ME0 = 0,
  ME1 = 1,
}

@thrift.TerseWrite
struct MyStruct {
  1: i32 field1;
}

union MyUnion {
  1: bool bool_field;
  2: byte byte_field;
  3: i16 short_field;
  4: i32 int_field;
  5: i64 long_field;
  6: float float_field;
  7: double double_field;
  8: string string_field;
  9: binary binary_field;
  10: MyEnum enum_field;
  11: list<i32> list_field;
  12: set<i32> set_field;
  13: map<i32, i32> map_field;
  14: MyStruct struct_field;
}

@thrift.TerseWrite
struct StructLevelTerseStruct {
  1: bool bool_field;
  2: byte byte_field;
  3: i16 short_field;
  4: i32 int_field;
  5: i64 long_field;
  6: float float_field;
  7: double double_field;
  8: string string_field;
  9: binary binary_field;
  10: MyEnum enum_field;
  11: list<i32> list_field;
  12: set<i32> set_field;
  13: map<i32, i32> map_field;
  14: MyStruct struct_field;
  15: MyUnion union_field;
}

struct FieldLevelTerseStruct {
  // terse-write fields
  @thrift.TerseWrite
  1: bool bool_field;
  @thrift.TerseWrite
  2: byte byte_field;
  @thrift.TerseWrite
  3: i16 short_field;
  @thrift.TerseWrite
  4: i32 int_field;
  @thrift.TerseWrite
  5: i64 long_field;
  @thrift.TerseWrite
  6: float float_field;
  @thrift.TerseWrite
  7: double double_field;
  @thrift.TerseWrite
  8: string string_field;
  @thrift.TerseWrite
  9: binary binary_field;
  @thrift.TerseWrite
  10: MyEnum enum_field;
  @thrift.TerseWrite
  11: list<i32> list_field;
  @thrift.TerseWrite
  12: set<i32> set_field;
  @thrift.TerseWrite
  13: map<i32, i32> map_field;
  @thrift.TerseWrite
  14: MyStruct struct_field;
  @thrift.TerseWrite
  15: MyUnion union_field;
}

// TODO(dokwon): Add support to py3 terse write with cpp.ref.
@thrift.TerseWrite
struct CppRefTerseStruct {
  @python.Py3Hidden
  @cpp.Ref{type = cpp.RefType.Unique}
  1: i32 unique_int_field;
  @python.Py3Hidden
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: i32 shared_int_field;
  @python.Py3Hidden
  @cpp.Ref{type = cpp.RefType.Shared}
  3: i32 shared_const_int_field;
  @thrift.InternBox
  4: MyStruct intern_boxed_field;
}

struct MixedFieldsStruct {
  @thrift.TerseWrite
  1: i32 terse_int_field;
  2: i32 def_int_field;
  3: optional i32 opt_int_field;
}

struct MixedFieldsStructWithCustomDefault {
  @thrift.TerseWrite
  1: i32 terse_int_field = 1;
  2: i32 def_int_field = 2;
  3: optional i32 opt_int_field = 3;
}

@thrift.TerseWrite
struct NestedMixedStruct {
  1: MixedFieldsStruct mixed_field;
  2: MixedFieldsStructWithCustomDefault mixed_field_with_custom_default;
}

struct EmptyStruct {}

@thrift.TerseWrite
struct MyStructWithCustomDefault {
  1: i32 field1 = 1;
}

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef MyStructWithCustomDefault MyStructWithCustomDefaultAdapted

@thrift.TerseWrite
struct TerseStructWithCustomDefault {
  1: bool bool_field = true;
  2: byte byte_field = 1;
  3: i16 short_field = 2;
  4: i32 int_field = 3;
  5: i64 long_field = 4;
  6: float float_field = 5.0;
  7: double double_field = 6.0;
  8: string string_field = "7";
  9: binary binary_field = "8";
  10: MyEnum enum_field = MyEnum.ME1;
  11: list<i32> list_field = [1];
  12: set<i32> set_field = [1];
  13: map<i32, i32> map_field = {1: 1};
  14: MyStructWithCustomDefault struct_field;
}

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef i32 AdaptedInteger
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef string AdaptedString
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef list<i32> AdaptedList

@thrift.TerseWrite
struct AdaptedFields {
  @python.Py3Hidden
  1: AdaptedInteger field1;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  2: i32 field2;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  3: i32 field3;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  4: AdaptedInteger field4;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  5: AdaptedInteger field5;
  6: string meta;
}

@thrift.TerseWrite
struct AdaptedStringFields {
  @python.Py3Hidden
  1: AdaptedString field1;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  2: string field2;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  3: string field3;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  4: AdaptedString field4;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  5: AdaptedString field5;
  6: string meta;
}

@thrift.TerseWrite
struct AdaptedListFields {
  @python.Py3Hidden
  1: AdaptedList field1;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  2: list<i32> field2;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  3: list<i32> field3;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  4: AdaptedList field4;
  @python.Py3Hidden
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  5: AdaptedList field5;
  6: string meta;
}

@thrift.TerseWrite
exception TerseException {
  @thrift.ExceptionMessage
  1: string msg;
}

@thrift.TerseWrite
struct ThreeLevelTerseStructs {
  1: TerseStructs field1;
  2: TerseStructs field2;
  3: TerseStructs field3;
  4: TerseStructs field4;
  5: TerseStructs field5;

  // disable field id packing
  101: TerseStructs field6;
  102: TerseStructs field7;
  103: TerseStructs field8;
  104: TerseStructs field9;
  105: TerseStructs field10;
}

@thrift.TerseWrite
struct TerseStructs {
  1: MyStruct field1;
  2: MyStruct field2;
  3: MyStruct field3;
}

@thrift.TerseWrite
struct TerseStructs1 {
  1: MyStruct field1;
}

@thrift.TerseWrite
struct TerseStructs2 {
  2: MyStruct field2;
}

@thrift.TerseWrite
struct TerseStructs3 {
  3: MyStruct field3;
}

@thrift.TerseWrite
struct TerseInternBoxedStructWithCustomDefault {
  @thrift.InternBox
  1: MyStructWithCustomDefault intern_boxed_field_with_custom_default;
  @python.Py3Hidden
  @thrift.InternBox
  2: MyStructWithCustomDefaultAdapted intern_boxed_field_with_custom_default_adapted;
}
