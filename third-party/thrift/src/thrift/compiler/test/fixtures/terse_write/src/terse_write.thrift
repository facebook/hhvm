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
include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.Experimental
package "facebook.com/thrift/test/terse_write"

enum MyEnum {
  ME0 = 0,
  ME1 = 1,
}

struct MyStruct {}

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
  11: list<i16> list_field;
  12: set<i16> set_field;
  13: map<i16, i16> map_field;
  14: MyStruct struct_field;
}

struct MyStructWithCustomDefault {
  1: i64 field1 = 1;
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
  11: list<i16> list_field;
  12: set<i16> set_field;
  13: map<i16, i16> map_field;
  14: MyStruct struct_field;
  15: MyUnion union_field;
}

struct FieldLevelTerseStruct {
  // terse-write fields
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
  @thrift.TerseWrite
  29: MyUnion terse_union_field;

  // non-terse-write fields
  15: bool bool_field;
  16: byte byte_field;
  17: i16 short_field;
  18: i32 int_field;
  19: i64 long_field;
  20: float float_field;
  21: double double_field;
  22: string string_field;
  23: binary binary_field;
  24: MyEnum enum_field;
  25: list<i16> list_field;
  26: set<i16> set_field;
  27: map<i16, i16> map_field;
  28: MyStruct struct_field;
  30: MyUnion union_field;
}

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\\Adapter1'}
@cpp.Adapter{name = "::my::Adapter"}
typedef i32 MyInteger

@thrift.TerseWrite
struct AdaptedFields {
  1: MyInteger field1;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = "::my::Adapter"}
  2: i32 field2;
  @cpp.Adapter{name = "::my::Adapter"}
  3: MyInteger field3;
}

@thrift.TerseWrite
exception TerseException {
  @thrift.ExceptionMessage
  1: string msg;
}
