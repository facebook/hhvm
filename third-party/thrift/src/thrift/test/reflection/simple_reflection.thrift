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
cpp_include "thrift/test/AdapterTest.h"

namespace cpp2 test_cpp2.simple_cpp_reflection

cpp_include "<deque>"

typedef binary (cpp2.type = "folly::IOBuf") IOBuf
typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr

enum enum1 {
  field0 = 0,
  field1 = 1,
  field2 = 2,
}

struct nested1 {
  1: optional float nfield00;
  2: optional byte nfield01;
}

struct smallstruct {
  1: i32 f1;
}

struct struct1 {
  1: required i32 field0;
  2: optional string field1;
  3: optional enum1 field2;
  4: required list<list<i32>> field3;
  5: required set<i32> field4;
  6: required map<i32, string> field5;
  7: required nested1 field6;
  8: i64 field7; # generate writer req/reader opt field
  9: string field8; # default requiredness type field
  # this generates an invalid thrift definition, so it won't be tested
  // 10: string field9 (cpp.ref = "true", cpp2.ref = "true")

  11: required list<bool> field10;
}

struct struct2 {
  1: required string req_string;
  2: optional string opt_string;
  3: string def_string;
}

@cpp.Adapter{name = "::apache::thrift::test::IdentityAdapter<std::int64_t>"}
typedef i64 AdaptedLong

union union1 {
  1: AdaptedLong field_i64;
  2: string field_string;
  66: list<i64> field_list_i64;
  99: list<string> field_list_string;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: string field_string_reference;
  999: binary field_binary;
  12: smallstruct field_smallstruct (cpp2.ref = "true");
}

struct struct3 {
  1: optional smallstruct opt_nested (cpp.ref = "true", cpp2.ref = "true");
  2: smallstruct def_nested (cpp.ref = "true", cpp2.ref = "true");
  3: required smallstruct req_nested (cpp.ref = "true", cpp2.ref = "true");
  4: optional smallstruct box_nested1 (thrift.box);
  5: optional smallstruct box_nested2 (thrift.box);
}

typedef map<i32, string> (cpp2.template = "std::unordered_map") unordered_map
typedef set<i32> (cpp2.template = "std::unordered_set") unordered_set
typedef list<i32> (cpp2.template = "std::deque") deque

struct struct4 {
  1: unordered_map um_field;
  2: unordered_set us_field;
  3: deque deq_field;
}

struct struct5 {
  1: binary def_field;
  2: IOBuf iobuf_field;
  3: IOBufPtr iobufptr_field;
}

struct struct5_workaround {
  1: binary def_field;
  2: IOBuf iobuf_field;
}

struct struct5_listworkaround {
  1: list<binary> binary_list_field;
  2: map<i32, binary> binary_map_field1;
}

struct struct6 {
  1: smallstruct def_field (cpp2.ref_type = "shared");
  2: optional smallstruct opt_field (cpp2.ref_type = "shared");
  3: required smallstruct req_field (cpp2.ref_type = "shared");
}

struct struct7 {
  1: i32 field1;
  2: string field2;
  3: enum1 field3;
  4: list<i32> field4;
  5: set<i32> field5;
  6: map<i32, string> field6;
  7: nested1 field7;
  8: i64 field8;
  9: string field9;
  10: smallstruct field10 (cpp2.ref_type = "shared");
  11: IOBuf field11;
  12: binary field12;
  13: optional smallstruct field13 (cpp2.ref = "true");
  14: required smallstruct field14 (cpp2.ref = "true");
  15: smallstruct field15 (cpp2.ref = "true");
  16: optional map<i32, string> field16 (cpp.ref);
}

struct struct8 {
  1: smallstruct def_field (cpp2.ref_type = "shared_const");
  2: optional smallstruct opt_field (cpp2.ref_type = "shared_const");
  3: required smallstruct req_field (cpp2.ref_type = "shared_const");
}

exception except1 {
  1: i32 field1;
  2: string field2;
}
