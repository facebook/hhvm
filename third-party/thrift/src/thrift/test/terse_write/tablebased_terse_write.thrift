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
include "thrift/annotation/cpp.thrift"

@thrift.Experimental
package "apache.org/thrift/test/tablebased_terse_write"

namespace cpp2 apache.thrift.test.tablebased_terse_write

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf
@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr

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

@thrift.TerseWrite
struct CustomStringFields {
  1: IOBuf iobuf_field;
  2: IOBufPtr iobuf_ptr_field;
}

struct EmptiableStruct {
  1: optional i32 opt_int_field;
  @thrift.TerseWrite
  2: i32 terse_int_field;
}

struct EmptiableStructField {
  @thrift.TerseWrite
  1: EmptiableStruct emptiable_struct;
}

struct MixedFieldsStruct {
  @thrift.TerseWrite
  1: i32 terse_int_field;
  2: i32 def_int_field;
  3: optional i32 opt_int_field;
}

@thrift.TerseWrite
struct NestedMixedStruct {
  1: MixedFieldsStruct mixed_field;
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
