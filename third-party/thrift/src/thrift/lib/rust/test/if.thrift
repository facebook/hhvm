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

struct WrapBinary {
  1: binary data;
} (rust.exhaustive)

struct WrapString {
  1: string data;
} (rust.exhaustive)

struct NonstandardCollectionTypes {
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

struct TestBytesShared {
  1: binary_1562 b;
}

// The following were automatically generated and may benefit from renaming.
typedef binary (rust.type = "Bytes") binary_1562
typedef binary (rust.type = "smallvec::SmallVec<[u8; 32]>") binary_459
typedef map<string, i64> (rust.type = "indexmap::IndexMap") map_string_i64_1188
typedef map<string, string> (rust.type = "HashMap") map_string_string_2454
typedef map<string, string> (
  rust.type = "indexmap::IndexMap",
) map_string_string_4179
typedef map<string, string> (rust.type = "BTreeMap") map_string_string_4844
typedef set<i64> (rust.type = "indexmap::IndexSet") set_i64_4482
typedef set<string> (rust.type = "HashSet") set_string_3129
typedef set<string> (rust.type = "indexmap::IndexSet") set_string_8508
typedef set<string> (rust.type = "BTreeSet") set_string_9948
