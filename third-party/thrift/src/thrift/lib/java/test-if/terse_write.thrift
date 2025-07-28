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
include "thrift/conformance/if/any.thrift"
include "thrift/annotation/java.thrift"

@thrift.Experimental
package "test.dev/thrift/lib/java/test/terse"

namespace java.swift com.facebook.thrift.test.terse

enum MyEnum {
  ME0 = 0,
  ME1 = 1,
}

struct MyStruct {
  1: i32 int_field;
  2: optional i64 long_field;
  3: required bool bool_field;
  4: SingleFieldStruct struct_field;
}

struct EmptyStruct {}

@thrift.TerseWrite
struct SingleFieldStruct {
  1: optional i32 int_field;
  2: string string_field;
  3: i32 int_terse_field;
}

@thrift.TerseWrite
struct Structv1 {
  1: string string_field;
}

@thrift.TerseWrite
struct Structv2 {
  1: string string_field;
  2: SingleFieldStruct inner_field;
}

@thrift.TerseWrite
struct NestedStruct {
  1: string string_field;
  2: SingleFieldStruct inner_field;
}

@thrift.TerseWrite
struct Structv3 {
  1: string string_field;
  2: NestedStruct nested_field;
}

exception MyException {
  1: string msg;
}

exception TerseException {
  @thrift.TerseWrite
  1: string msg;
  @thrift.TerseWrite
  2: i32 code;
  3: string reason;
}

//@thrift.TerseWrite
// Terse write is not supported in Unions.
//union TerseUnion {
//  1: i32 int_field1;
//  2: i32 int_field2;
//  3: i32 int_field3;
//}

union MyUnion {
  1: i32 int_field1;
  2: i32 int_field2;
  3: i32 int_field3;
}

@thrift.TerseWrite
struct InnerTerseStruct {
  1: bool bool_field;
  2: byte byte_field;
}

struct MyStructWithCustomDefault {
  1: i64 field1 = 1;
}

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ByteToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef byte adaptedByte

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
  11: list<i16> list_field;
  12: set<i16> set_field;
  13: map<i16, i16> map_field;
  14: MyStruct struct_field;
  15: InnerTerseStruct inner_field;
  16: MyUnion union_field;
  17: TerseException exception_field;
  18: adaptedByte adapted_byte_field;
}

struct FieldLevelTerseStruct {
  @thrift.TerseWrite
  1: bool terse_bool_field;
  @thrift.TerseWrite
  2: byte terse_byte_field;
  @thrift.TerseWrite
  3: i16 terse_short_field;
  @thrift.TerseWrite
  4: i32 terse_int_field;
  @thrift.TerseWrite
  5: i64 terse_long_field;
  @thrift.TerseWrite
  6: float terse_float_field;
  @thrift.TerseWrite
  7: double terse_double_field;
  @thrift.TerseWrite
  8: string terse_string_field;
  @thrift.TerseWrite
  9: binary terse_binary_field;
  @thrift.TerseWrite
  10: MyEnum terse_enum_field;
  @thrift.TerseWrite
  11: list<i16> terse_list_field;
  @thrift.TerseWrite
  12: set<i16> terse_set_field;
  @thrift.TerseWrite
  13: map<i16, i16> terse_map_field;
  @thrift.TerseWrite
  14: MyStruct terse_struct_field;

  // non-terse-write fields
  15: optional bool bool_field;
  16: byte byte_field;
  17: i16 short_field;
  18: i32 int_field;
  19: optional i64 long_field;
  20: float float_field;
  21: double double_field;
  22: string string_field;
  23: optional binary binary_field;
  24: MyEnum enum_field;
  25: optional list<i16> list_field;
  26: set<i16> set_field;
  27: map<i16, i16> map_field;
  28: MyStruct struct_field;
  29: optional list<MyStruct> struct_list_field;
  30: optional list<list<bool>> bool_list_list_field;
  31: optional list<list<list<bool>>> bool_list_list_list_field;
  32: EmptyStruct empty_field;
  33: SingleFieldStruct single_field;
  34: MyUnion union_field;
}

@thrift.TerseWrite
struct TerseStructSingleField {
  1: i32 int_field;
}

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.UnpooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary UnpooledByteBuf

@thrift.TerseWrite
struct TerseStructWithPrimitiveTypeAdapter {
  1: i32 int_field;
  2: UnpooledByteBuf binary_field;
}

struct TopLevelStruct {
  @thrift.TerseWrite
  1: i32 int_field;
  @thrift.TerseWrite
  2: TerseStructWithPrimitiveTypeAdapter inner_field;
}

@thrift.TerseWrite
struct TerseStructWithStructTypeAdapter {
  1: i32 int_field;
  // Any is struct and have required fields
  2: any.LazyAny any_field;
}

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.DateTypeAdapter",
  typeClassName = "java.util.Date",
}
typedef i64 Date

struct TerseStructWithDateAdapter {
  1: string string_field;
  @thrift.TerseWrite
  2: Date date_field;
}

struct TestV0 {
  1: bool boolean_field;
  2: byte byte_field;
  3: i16 short_field;
}

struct TestV1 {
  1: bool boolean_field;
  2: byte byte_field;
  3: i16 short_field;
}

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.BooleanToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef bool adaptedBoolean

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ShortToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef i16 adaptedShort

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.IntToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef i32 Integer
typedef Integer adaptedInt

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.LongToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef i64 adaptedLong

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.common.RetainedSlicedPooledByteBufTypeAdapter",
  typeClassName = "io.netty.buffer.ByteBuf",
}
typedef binary SlicedByteBuf

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.ListToHexTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef list<binary> adaptedBinaryList

typedef adaptedInt doubleTypedefInt

@thrift.TerseWrite
struct AdaptedTerseStruct {
  1: adaptedBoolean adaptedBoolean_field;
  3: adaptedShort adaptedShort_field;
  4: adaptedInt adaptedInt_field;
  5: adaptedLong adaptedLong_field;
  10: SlicedByteBuf b1;
  21: adaptedBinaryList adaptedBinaryList_field;
  101: optional adaptedBoolean optionalAdaptedBoolean_field;
  102: optional SlicedByteBuf optional_b1;
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.adapter.test.StringToLongTypeAdapter",
    typeClassName = "java.lang.Long",
  }
  204: adaptedInt doubleAdaptedInt_field;
  206: doubleTypedefInt doubleTypedefAdaptedInt_field;
}

@thrift.TerseWrite
struct TerseStruct {
  1: bool boolean_field;
  3: i16 short_field;
  4: i32 int_field;
  5: i64 long_field;
  10: binary b1;
  21: list<binary> binaryList_field;
  101: optional bool optionalBoolean_field;
  102: optional binary optional_b1;
  204: i32 int_field2;
  205: i32 int_default2;
  206: i32 doubleTypedefInt_field;
}
