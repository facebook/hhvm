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

@thrift.Experimental
package "facebook.com/thrift/test/fixtures/terse_write/deprecated"

enum MyEnum {
  ME0 = 0,
  ME1 = 1,
}

struct MyStruct {}
union MyUnion {}

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

  // non-terse-write fields
  @cpp.DeprecatedTerseWrite
  15: bool bool_field;
  @cpp.DeprecatedTerseWrite
  16: byte byte_field;
  @cpp.DeprecatedTerseWrite
  17: i16 short_field;
  @cpp.DeprecatedTerseWrite
  18: i32 int_field;
  @cpp.DeprecatedTerseWrite
  19: i64 long_field;
  @cpp.DeprecatedTerseWrite
  20: float float_field;
  @cpp.DeprecatedTerseWrite
  21: double double_field;
  @cpp.DeprecatedTerseWrite
  22: string string_field;
  @cpp.DeprecatedTerseWrite
  23: binary binary_field;
  @cpp.DeprecatedTerseWrite
  24: MyEnum enum_field;
  @cpp.DeprecatedTerseWrite
  25: list<i16> list_field;
  @cpp.DeprecatedTerseWrite
  26: set<i16> set_field;
  @cpp.DeprecatedTerseWrite
  27: map<i16, i16> map_field;
  28: MyStruct struct_field;
  29: MyUnion union_field;
  @cpp.DeprecatedTerseWrite
  @cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
  30: byte iobuf_ptr_field;
}

struct CppRefStructFields {
  @cpp.AllowLegacyNonOptionalRef
  @cpp.DeprecatedTerseWrite
  @cpp.AllowLegacyDeprecatedTerseWritesRef
  @cpp.Ref{type = cpp.RefType.Unique}
  1: i32 primitive_ref_field;
  @cpp.AllowLegacyNonOptionalRef
  @cpp.DeprecatedTerseWrite
  @cpp.AllowLegacyDeprecatedTerseWritesRef
  @cpp.Ref{type = cpp.RefType.Unique}
  2: MyStruct struct_ref_field;
}

struct DeprecatedTerseWriteWithCustomDefault {
  @cpp.DeprecatedTerseWrite
  1: bool bool_field = true;
  @cpp.DeprecatedTerseWrite
  2: byte byte_field = 42;
  @cpp.DeprecatedTerseWrite
  3: i16 short_field = 42;
  @cpp.DeprecatedTerseWrite
  4: i32 int_field = 42;
  @cpp.DeprecatedTerseWrite
  5: i64 long_field = 42;
  @cpp.DeprecatedTerseWrite
  6: float float_field = 42.0;
  @cpp.DeprecatedTerseWrite
  7: double double_field = 42.0;
  @cpp.DeprecatedTerseWrite
  8: string string_field = "hello";
  @cpp.DeprecatedTerseWrite
  9: binary binary_field = "world";
  @cpp.DeprecatedTerseWrite
  10: MyEnum enum_field = MyEnum.ME1;
  @cpp.DeprecatedTerseWrite
  11: list<i16> list_field = [1];
  @cpp.DeprecatedTerseWrite
  12: set<i16> set_field = [1];
  @cpp.DeprecatedTerseWrite
  13: map<i16, i16> map_field = {1: 1};
}

struct DeprecatedTerseWriteWithRedundantCustomDefault {
  @cpp.DeprecatedTerseWrite
  1: bool bool_field = false;
  @cpp.DeprecatedTerseWrite
  2: byte byte_field = 0;
  @cpp.DeprecatedTerseWrite
  3: i16 short_field = 0;
  @cpp.DeprecatedTerseWrite
  4: i32 int_field = 0;
  @cpp.DeprecatedTerseWrite
  5: i64 long_field = 0;
  @cpp.DeprecatedTerseWrite
  6: float float_field = 0.0;
  @cpp.DeprecatedTerseWrite
  7: double double_field = 0.0;
  @cpp.DeprecatedTerseWrite
  8: string string_field = "";
  @cpp.DeprecatedTerseWrite
  9: binary binary_field = "";
  @cpp.DeprecatedTerseWrite
  10: MyEnum enum_field = MyEnum.ME0;
  @cpp.DeprecatedTerseWrite
  11: list<i16> list_field = [];
  @cpp.DeprecatedTerseWrite
  12: set<i16> set_field = [];
  @cpp.DeprecatedTerseWrite
  13: map<i16, i16> map_field = {};
}
