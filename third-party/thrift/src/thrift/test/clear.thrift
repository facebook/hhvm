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

namespace cpp2 apache.thrift.test

package "facebook.com/thrift/test/clear"

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

cpp_include "thrift/test/AdapterTest.h"

enum MyEnum {
  ME0 = 0,
  ME1 = 1,
}

struct MyStruct {
  1: i32 int_field;
}

struct DefaultMyStruct {
  1: i32 int_field = 1;
}

struct StructWithNoDefaultStruct {
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
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  15: MyStruct ref_field;
}

struct StructWithDefaultStruct {
  1: bool bool_field = true;
  2: byte byte_field = 1;
  3: i16 short_field = 1;
  4: i32 int_field = 1;
  5: i64 long_field = 1;
  6: float float_field = 1.0;
  7: double double_field = 1.0;
  8: string string_field = "1";
  9: binary binary_field = "1";
  10: MyEnum enum_field = MyEnum.ME1;
  11: list<i16> list_field = [1];
  12: set<i16> set_field = [1];
  13: map<i16, i16> map_field = {1: 1};
  14: DefaultMyStruct struct_field;
}

struct ThriftClearTestStruct {
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  1: i64 data;
  2: string meta;
}

struct AdapterClearTestStruct {
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContextOptimized"}
  1: i64 data;
  2: string meta;
}

@thrift.Experimental
@thrift.TerseWrite
struct TerseWriteField {
  1: i32 field_1;
}

struct OptionalField {
  1: optional i32 optional_i32;
  @thrift.Box
  2: optional i32 boxed_i32;
  @cpp.Ref{type = cpp.RefType.Shared}
  3: optional i32 shared_i32;
  @cpp.Ref{type = cpp.RefType.Unique}
  4: optional i32 unique_i32;
}
