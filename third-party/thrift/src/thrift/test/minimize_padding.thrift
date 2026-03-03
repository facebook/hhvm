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

@thrift.AllowLegacyMissingUris
package;

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
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required i16 a;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required bool b;
  @thrift.AllowUnsafeRequiredFieldQualifier
  3: required i32 c;
  @thrift.AllowUnsafeRequiredFieldQualifier
  4: required byte d;
  @thrift.AllowUnsafeRequiredFieldQualifier
  5: required i64 e;
  @thrift.AllowUnsafeRequiredFieldQualifier
  6: required test_enum f;
  @thrift.AllowUnsafeRequiredFieldQualifier
  7: required double g;
  @thrift.AllowUnsafeRequiredFieldQualifier
  8: required float h;
  @thrift.AllowUnsafeRequiredFieldQualifier
  9: required string i;
  @thrift.AllowUnsafeRequiredFieldQualifier
  10: required byte j;
  @thrift.AllowUnsafeRequiredFieldQualifier
  11: required list<byte> k;
  @thrift.AllowUnsafeRequiredFieldQualifier
  12: required byte l;
  @thrift.AllowUnsafeRequiredFieldQualifier
  13: required set<byte> m;
  @thrift.AllowUnsafeRequiredFieldQualifier
  14: required byte n;
  @thrift.AllowUnsafeRequiredFieldQualifier
  15: required map<byte, byte> o;

  // Add extra fields to make the size of this struct a multiple of 8 (assuming
  // containers' alignment is 8) and reducing the room for reordering error.
  @thrift.AllowUnsafeRequiredFieldQualifier
  16: required byte p;
  @thrift.AllowUnsafeRequiredFieldQualifier
  17: required i32 q;
}

struct small_align {
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required byte a;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required byte b;
}

struct big_align {
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required byte a;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required i32 b;
}

@cpp.MinimizePadding
struct nonoptimal_struct {
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required byte small;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required big_align big;
  @thrift.AllowUnsafeRequiredFieldQualifier
  3: required small_align medium;
}

@cpp.MinimizePadding
struct nonoptimal_struct_with_structured_annotation {
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required byte small;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required big_align big;
  @thrift.AllowUnsafeRequiredFieldQualifier
  3: required small_align medium;
}

@cpp.MinimizePadding
struct nonoptimal_struct_with_custom_type {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required byte small;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required big_align big;
  @thrift.AllowUnsafeRequiredFieldQualifier
  3: required small_align medium;
}

@cpp.MinimizePadding
struct same_sizes {
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required i32 a;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required i32 b;
  @thrift.AllowUnsafeRequiredFieldQualifier
  3: required i32 c;
  @thrift.AllowUnsafeRequiredFieldQualifier
  4: required i32 d;
}

@cpp.MinimizePadding
struct ref_type {
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required byte a;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required byte b;
  @thrift.AllowUnsafeRequiredFieldQualifier
  3: required byte c;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  @thrift.AllowUnsafeRequiredFieldQualifier
  4: required byte d;
}

@cpp.MinimizePadding
struct nonoptimal_struct_noexcept_move {
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required byte small;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required big_align big;
  @thrift.AllowUnsafeRequiredFieldQualifier
  3: required small_align medium;
}

@cpp.MinimizePadding
struct nonoptimal_large_struct_noexcept_move {
  @thrift.AllowUnsafeRequiredFieldQualifier
  1: required byte small;
  @thrift.AllowUnsafeRequiredFieldQualifier
  2: required big_align big;
  @thrift.AllowUnsafeRequiredFieldQualifier
  3: required small_align medium;
  @thrift.AllowUnsafeRequiredFieldQualifier
  4: required string mystring;
  @thrift.AllowUnsafeRequiredFieldQualifier
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
