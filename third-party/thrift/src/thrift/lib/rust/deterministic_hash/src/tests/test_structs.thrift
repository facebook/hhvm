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

namespace cpp2 apache.thrift.op

struct SimpleStruct1 {
  1: byte byte_; // 8
  2: i16 i_16; // 12
  3: i32 i_32; // 1
  4: i64 i_64; // 2
  5: float f_32; // 1.0
  6: double f_64; // 2.0
  7: string str_with_value; // "abc"
  8: string str_empty; // ""
  9: optional string str_not_set; // not set
  10: bool bool_true; // true
  11: bool bool_false; // false
  12: binary binary_; // bytes[1,2,3]
}

// There is no unsigned types in thrift so we use i16 instead
const list<i16> SimpleStructSha256Hash = [
  52,
  215,
  208,
  57,
  180,
  23,
  19,
  92,
  11,
  109,
  89,
  230,
  17,
  54,
  111,
  147,
  251,
  244,
  166,
  160,
  19,
  22,
  250,
  29,
  187,
  246,
  14,
  18,
  197,
  199,
  190,
  37,
];

struct ComplexStruct {
  1: list<string> l;
  2: set<string> s;
  3: map<string, string> m;
  4: map<string, list<i32>> ml;
  5: map<string, map<i32, i32>> mm;
}

const list<i16> ComplexStructSha256Hash = [
  215,
  3,
  60,
  5,
  187,
  190,
  35,
  151,
  36,
  121,
  90,
  142,
  117,
  193,
  153,
  144,
  17,
  19,
  156,
  131,
  209,
  173,
  91,
  93,
  59,
  156,
  245,
  18,
  29,
  76,
  76,
  172,
];
