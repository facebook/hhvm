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

namespace rust interface

include "thrift/annotation/rust.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@rust.Exhaustive
struct WrapBinary {
  1: binary data;
}

@rust.Exhaustive
struct WrapString {
  1: string data;
}

// The field types here are each of a form `@rust.Type{name = "C"} typedef A B`.
struct TypedefNondefaultTypes {
  10: set<string> defaultset;
  11: set_string_9948 btreeset;
  12: set_string_3129 hashset;
  13: set_string_8508 indexset_a;
  14: set_string_8508 indexset_b;
  15: set_i64_4482 indexset_c;

  20: map<string, string> defaultmap;
  21: map_string_string_4844 btreemap;
  22: map_string_string_2454 hashmap;
  23: map_string_string_4179 indexmap_a;
  24: map_string_string_4179 indexmap_b;
  25: map_string_i64_1188 indexmap_c;

  30: binary_459 bin_smallvec;
  31: binary_1562 bin_bytes;
}

enum TestEnum {
  FOO = 1,
  BAR = 2,
  BAZ = 3,
}

enum TestEnumEmpty {
}

struct TestSkipV1 {}

struct TestSkipV2 {
  1: list<string> l;
  2: set<string> s;
  3: map<string, string> m;
}

struct TestSkipMinimal {
  20: byte b;
}

struct TestSkipNested {
  1: list<i32> l;
  2: set<byte> s;
  3: map<string, string> m;
}

struct TestSkipNestedDeep {
  1: TestSkipNested nested;
  2: list<TestSkipNested> nested_list;
}

struct TestSkipFull {
  1: list<i32> l;
  2: set<byte> s;
  3: map<string, string> m_str;
  4: TestSkipNested nested;
  5: list<TestSkipNested> nested_list;
  6: bool bl;
  7: map<i16, i64> m_int;
  8: map<i32, TestSkipNested> m_nested;
  9: list<TestSkipNestedDeep> l_nested2;
  10: list<double> l_double;
  11: list<float> l_float;

  20: byte b;
}

struct TestBytesShared {
  1: binary_1562 b;
}

@rust.Type{name = "Bytes"}
typedef binary binary_1562
@rust.Type{name = "smallvec::SmallVec<[u8; 32]>"}
typedef binary binary_459
@rust.Type{name = "indexmap::IndexMap"}
typedef map<string, i64> map_string_i64_1188
@rust.Type{name = "HashMap"}
typedef map<string, string> map_string_string_2454
@rust.Type{name = "indexmap::IndexMap"}
typedef map<string, string> map_string_string_4179
@rust.Type{name = "BTreeMap"}
typedef map<string, string> map_string_string_4844
@rust.Type{name = "indexmap::IndexSet"}
typedef set<i64> set_i64_4482
@rust.Type{name = "HashSet"}
typedef set<string> set_string_3129
@rust.Type{name = "indexmap::IndexSet"}
typedef set<string> set_string_8508
@rust.Type{name = "BTreeSet"}
typedef set<string> set_string_9948

typedef binary binary_t
typedef map<string, string> map_string_string_t

@rust.Type{name = "u8"}
typedef byte u8
@rust.Type{name = "u16"}
typedef i16 u16
@rust.Type{name = "u32"}
typedef i32 u32
@rust.Type{name = "u64"}
typedef i64 u64

struct TestUnsignedStruct {
  1: u8 b;
  2: u16 c;
  3: u32 d;
  4: u64 e;
}
