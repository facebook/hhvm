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

typedef binary (cpp.type = "folly::IOBuf") IOBuf

struct OneOfEach {
  1: bool im_true;
  2: bool im_false;
  3: byte a_bite = 127;
  4: i16 integer16 = 32767;
  5: i32 integer32;
  6: i64 integer64 = 1000000000;
  7: double double_precision;
  8: string some_characters;
  9: string zomg_unicode;
  10: bool what_who;
  11: binary base64;
  12: list<byte> byte_list = [1, 2, 3];
  13: list<i16> i16_list = [1, 2, 3];
  14: list<i64> i64_list = [1, 2, 3];
  15: IOBuf io_buf;
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
  6: OneOfEach ref_field (cpp.ref = "true");
}

struct StructWithAUnion {
  1: TestUnion test_union;
}

struct NonCopyableStruct {
  1: i64 num;
} (cpp.noncopyable)

union NonCopyableUnion {
  1: i32 a;
  2: IOBuf buf;
  3: NonCopyableStruct ncs;
} (cpp.noncopyable, cpp.noncomparable)

union NoExceptMoveUnion {
  1: string string_field;
  2: i32 i32_field;
  3: OneOfEach struct_field;
}

union CppRefContainers {
  1: list<CppRefContainers> values (cpp.ref);
  2: string data;
}
