/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 facebook.thrift.test.tablebased

include "thrift/test/tablebased/include_tablebased.thrift"

typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr

enum Enum {
  A = 1,
  B = 2,
}

struct StructA {
  1: optional i64 i64_field;
  2: optional string opt_str_field;
  3: optional StructB struct_field;
  5: optional list<string> list_field;
  10: optional map<string, i64> map_field;
  11: string str_field;
  12: binary bin_field;
  13: Enum enum_field;
}

struct StructWithRef {
  1: optional StructB shared_struct_field (cpp.ref_type = "shared_const");
  2: optional list<string> shared_list_field (cpp.ref_type = "shared_const");
  3: optional i16 shared_i16_field (cpp.ref_type = "shared_const");
  4: optional i32 unique_i32_field (cpp.ref_type = "unique");
}

struct StructWithCppType {
  1: optional map<string, StructA> (
    cpp.type = "std::unordered_map<std::string, StructA>",
  ) field;
}

struct StructB {
  1: string str_field;
  2: optional i64 i64_field;
  3: optional IOBufPtr iobufptr_field;
  5: optional list<i64> list_field (cpp.ref_type = "shared");
  6: i32 i32_field;
  7: i16 i16_field;
  8: byte byte_field;
  9: bool bool_field;
  10: set<i32> set_field;
  11: string iobuf_field (cpp.type = "folly::IOBuf");
  12: double double_field;
  13: float float_field;
}

struct StructWithInclude {
  1: optional include_tablebased.IncludedStruct field;
}

union Union {
  1: StructA a_field;
  2: StructB b_field;
  3: string str_field;
}

union UnionWithRef {
  1: StructB simple_field;
  2: StructA unique_field (cpp.ref_type = "unique");
  4: StructA shared_field (cpp2.ref_type = "shared");
  3: StructA shared_const_field (cpp.ref_type = "shared_const");
}

union TestUnion {
  1: string string_field;
  2: float float_field;
}

struct TestStructWithUnion {
  1: TestUnion union_field;
}
