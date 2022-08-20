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

namespace cpp thrift.test.debug
namespace py thrift.test.UnionTest
namespace java thrift.test.union

struct OneOfEach {
  // make at least one field of a struct contained in a union required
  // to test exception handling in union code
  1: required bool im_true;
  2: bool im_false;
  3: byte a_bite = 100;
  4: i16 integer16 = 23000;
  5: i32 integer32;
  6: i64 integer64 = 10000000000;
  7: double double_precision;
  8: string some_characters;
  9: string zomg_unicode;
  10: bool what_who;
  11: binary base64;
  12: list<byte> byte_list = [1, 2, 3];
  13: list<i16> i16_list = [1, 2, 3];
  14: list<i64> i64_list = [1, 2, 3];
}

struct RandomStuff {
  1: i32 a;
  2: i32 b;
  3: i32 c;
  4: i32 d;
  5: list<i32> myintlist;
  7: i64 bigint;
  8: double triple;
}

union TestUnion {
  /**
   * A doc string
   */
  1: string string_field;
  2: i32 i32_field;
  3: OneOfEach struct_field;
  4: list<RandomStuff> struct_list;
  5: i32 other_i32_field;
}

struct StructWithAUnion {
  1: TestUnion test_union;
  2: TestUnion test_union_ref (cpp.ref = "true");
}

struct StructWithUnionAndOther {
  1: TestUnion test_union;
  2: string string_field;
}

const list<StructWithUnionAndOther> NESTED = [
  {"test_union": {"i32_field": 3}},
  {"string_field": "hello"},
  {"test_union": {"other_i32_field": 4}},
];

struct StructWithDoubleUnderscoreField {
  1: i32 __field;
  2: i32 field;
}
