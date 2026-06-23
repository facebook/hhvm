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
 *
 */

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/hack.thrift"
package "meta.com/thrift/core/protocol/_tests_/protocol_test"

namespace hack ""

@thrift.TerseWrite
@thrift.Experimental
struct SimpleStruct {
  1: bool a_bool;
  2: byte a_byte;
  3: i16 a_i16;
  4: i32 a_i32;
  5: i64 a_i64;
  6: float a_float;
  7: double a_double;
  8: string a_string;
  9: binary a_binary;
}

union SimpleUnion {
  1: bool a_bool;
  2: byte a_byte;
}

struct ObjectKeyStruct {
  1: i64 id;
  2: string name;
}

@hack.Wrapper{name = '\MyStructWrapper'}
struct ObjectKeyWrappedValueStruct {
  1: i64 id;
  2: string name;
}

@hack.Adapter{name = '\ObjectKeyStructShapeAdapter'}
typedef ObjectKeyStruct AdaptedObjectKeyStruct

@hack.Adapter{name = '\AdapterTestIntToString'}
typedef i32 AdaptedObjectKeyI32

union ObjectKeyValue {
  1: bool bool_value;
  2: i64 int_value;
  3: string string_value;
  4: ObjectKeyStruct struct_value;
  5: list<ObjectKeyValue> list_value;

  @thrift.AllowUnsafeNonSealedKeyType
  6: set<ObjectKeyValue> set_value;

  @thrift.AllowUnsafeNonSealedKeyType
  7: map<ObjectKeyValue, ObjectKeyValue> map_value;
}

struct ObjectKeyContainerStruct {
  1: set<ObjectKeyStruct> struct_set;
  2: map<ObjectKeyStruct, string> struct_map;
  3: set<float> float_set;
  4: set<bool> bool_set;
  5: map<bool, string> bool_map;

  @thrift.AllowUnsafeNonSealedKeyType
  6: set<ObjectKeyValue> value_set;

  @thrift.AllowUnsafeNonSealedKeyType
  7: map<ObjectKeyValue, i64> value_map;

  8: string normal_field;
  9: map<string, i64> normal_map;
}

struct ObjectKeyAdaptedContainerStruct {
  1: map<AdaptedObjectKeyStruct, string> adapted_struct_map;
  2: set<AdaptedObjectKeyStruct> adapted_struct_set;
  3: map<AdaptedObjectKeyI32, string> adapted_i32_map;
  4: set<AdaptedObjectKeyI32> adapted_i32_set;
  5: map<ObjectKeyStruct, AdaptedObjectKeyStruct> adapted_struct_value_map;
  6: map<
    AdaptedObjectKeyStruct,
    AdaptedObjectKeyStruct
  > adapted_struct_key_value_map;
}

struct ObjectKeyWrappedValueContainerStruct {
  1: map<ObjectKeyStruct, ObjectKeyWrappedValueStruct> wrapped_struct_value_map;
}

struct ContainerStruct {
  1: list<bool> bool_list;
  2: list<byte> byte_list;
  3: list<i16> i16_list;
  4: list<i32> i32_list;
  5: list<i64> i64_list;
  6: list<float> float_list;
  7: list<double> double_list;
  8: list<string> string_list;
  9: list<binary> binary_list;

  11: set<byte> byte_set;
  12: set<i16> i16_set;
  13: set<i32> i32_set;
  14: set<i64> i64_set;
  17: set<string> string_set;
  18: set<binary> binary_set;

  20: map<byte, bool> byte_bool_map;
  21: map<i32, i16> i32_i16_map;
  22: map<string, i64> string_i64_map;

  30: list<map<byte, bool>> list_byte_bool_map;
}

@thrift.TerseWrite
@thrift.Experimental
struct ComplexStruct {
  1: SimpleStruct a_simple_struct;
  2: ContainerStruct a_container_struct;
  3: SimpleUnion a_simple_union;

  10: list<SimpleStruct> simple_struct_list;
  11: list<ContainerStruct> container_struct_list;
  14: map<string, SimpleStruct> string_simple_struct_map;
  15: map<string, ContainerStruct> string_container_struct_map;
}

@thrift.TerseWrite
@thrift.Experimental
struct TestTerseWriteStruct {
  1: bool a_bool;
  2: byte a_byte;
  3: i16 a_i16;
  4: i32 a_i32;
  5: i64 a_i64;
  6: float a_float;
  7: double a_double;
  8: string a_string;
  9: binary a_binary;
  10: list<SimpleStruct> list_of_simple_structs;
  11: set<i32> set_of_ints;
  12: map<string, ComplexStruct> map_of_str_to_struct;
}

struct CorruptedDataStruct1 {
  1: list<string> string_list;
  2: set<string> string_set;
  3: map<string, i64> string_i64_map;
}

struct CorruptedDataStruct2 {
  1: list<i64> i64_list;
  2: set<i64> i64_set;
  3: map<i64, i64> i64_i64_map;
}
