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
include "thrift/test/terse_write/terse_write.thrift"

@thrift.Experimental
package "apache.org/thrift/test/deprecated_terse_write"

namespace cpp2 apache.thrift.test.deprecated_terse_write

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
  10: terse_write.MyEnum enum_field;
  11: list<i32> list_field;
  12: set<i32> set_field;
  13: map<i32, i32> map_field;
  14: terse_write.MyStruct struct_field;
  15: terse_write.MyUnion union_field;
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
  10: terse_write.MyEnum enum_field;
  @thrift.TerseWrite
  11: list<i32> list_field;
  @thrift.TerseWrite
  12: set<i32> set_field;
  @thrift.TerseWrite
  13: map<i32, i32> map_field;
  @thrift.TerseWrite
  14: terse_write.MyStruct struct_field;
  @thrift.TerseWrite
  15: terse_write.MyUnion union_field;
}
