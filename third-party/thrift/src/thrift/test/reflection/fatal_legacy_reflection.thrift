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

cpp_include "thrift/test/reflection/fatal_legacy_reflection_types.h"

namespace cpp2 apache.thrift.test

typedef i32 (cpp.type = 'CppHasANumber', cpp.indirection) HasANumber

enum SampleEnum {
  kSampleEnumFoo = 0,
  kSampleEnumBar = 1,
}

struct SampleSubStruct {
  1: string string_sub_field;
}

union SampleSubUnion {
  1: string string_sub_field;
}

struct SampleStruct {
  1: bool bool_field;
  2: byte byte_field;
  3: i16 i16_field;
  4: i32 i32_field;
  5: i64 i64_field;
  6: double double_field;
  7: float float_field;
  8: binary binary_field;
  9: string string_field;
  10: SampleEnum enum_field;
  11: SampleSubStruct struct_field;
  12: SampleSubUnion union_field;
  13: list<i16> list_i16_field;
  14: set<i32> set__i32_field;
  15: map<i64, string> map_i64_string_field;
  16: list<bool> list_bool_field;
  17: list<byte> list_byte_field;
  18: list<double> list_double_field;
  19: list<float> list_float_field;
  20: list<binary> list_binary_field;
  21: string annotated_string_field (ann_key = 'ann_value');
  22: HasANumber i32_indirection_field;
  23: SampleSubStruct struct_ref_field (cpp.ref);
  24: SampleSubStruct struct_unique_field (cpp.ref_type = 'unique');
  25: SampleSubStruct struct_shared_field (cpp.ref_type = 'shared');
  26: SampleSubStruct struct_shared_const_field (cpp.ref_type = 'shared_const');
  31: optional SampleSubStruct struct_box_field (thrift.box);

  // integer custom types
  27: byte (cpp.type = "std::uint8_t") ubyte_field;
  28: i16 (cpp.type = "std::uint16_t") ui16_field;
  29: i32 (cpp.type = "std::uint32_t") ui32_field;
  30: i64 (cpp.type = "std::uint64_t") ui64_field;
}
