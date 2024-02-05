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
cpp_include "thrift/test/AdapterTest.h"

namespace cpp2 apache.thrift.test

@cpp.MinimizePadding
struct out_of_order_struct {
  1: empty field;
}

@cpp.MinimizePadding
struct empty {}

enum test_enum {
  foo = 0,
}

@cpp.EnumType{type = cpp.EnumUnderlyingType.I8}
enum byte_enum {
}

@cpp.EnumType{type = cpp.EnumUnderlyingType.I16}
enum short_enum {
}

@cpp.EnumType{type = cpp.EnumUnderlyingType.U32}
enum unsigned_int_enum {
}

@cpp.MinimizePadding
struct nonoptimal {
  1: required i16 a;
  2: required bool b;
  3: required i32 c;
  4: required byte d;
  5: required i64 e;
  6: required test_enum f;
  7: required double g;
  8: required float h;
  9: required string i;
  10: required byte j;
  11: required list<byte> k;
  12: required byte l;
  13: required set<byte> m;
  14: required byte n;
  15: required map<byte, byte> o;

  // Add extra fields to make the size of this struct a multiple of 8 (assuming
  // containers' alignment is 8) and reducing the room for reordering error.
  16: required byte p;
  17: required i32 q;
}

struct small_align {
  1: required byte a;
  2: required byte b;
}

struct big_align {
  1: required byte a;
  2: required i32 b;
}

@cpp.MinimizePadding
struct nonoptimal_struct {
  1: required byte small;
  2: required big_align big;
  3: required small_align medium;
}

@cpp.MinimizePadding
struct nonoptimal_struct_with_structured_annotation {
  1: required byte small;
  2: required big_align big;
  3: required small_align medium;
}

@cpp.MinimizePadding
struct nonoptimal_struct_with_custom_type {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  1: required byte small;
  2: required big_align big;
  3: required small_align medium;
}

@cpp.MinimizePadding
struct same_sizes {
  1: required i32 a;
  2: required i32 b;
  3: required i32 c;
  4: required i32 d;
}

@cpp.MinimizePadding
struct ref_type {
  1: required byte a;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: required byte b;
  3: required byte c;
  @cpp.Ref{type = cpp.RefType.Unique}
  4: required byte d;
}

@cpp.MinimizePadding
struct nonoptimal_struct_noexcept_move {
  1: required byte small;
  2: required big_align big;
  3: required small_align medium;
}

@cpp.MinimizePadding
struct nonoptimal_large_struct_noexcept_move {
  1: required byte small;
  2: required big_align big;
  3: required small_align medium;
  4: required string mystring;
  5: required i32 a;
}

@thrift.Experimental
@thrift.TerseWrite
struct enums {
  1: byte_enum byte_enum_field;
  2: unsigned_int_enum unsigned_int_enum_field;
  3: short_enum short_enum_field;
}

@thrift.Experimental
@cpp.MinimizePadding
@thrift.TerseWrite
struct minimized_enums {
  1: byte_enum byte_enum_field;
  2: unsigned_int_enum unsigned_int_enum_field;
  3: short_enum short_enum_field;
}
