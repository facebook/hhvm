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

cpp_include "folly/sorted_vector_types.h"

include "thrift/annotation/cpp.thrift"

enum enum1 {
  field0 = 0,
  field1 = 1,
  field2 = 2,
}

const enum1 e_1 = field0;
const enum1 e_2 = field2;

const i32 i_1 = 72;
const i32 i_2 = 99;

const string str_e = "";
const string str_1 = "hello";
const string str_2 = "world";
const string str_3 = "'";
const string str_4 = '"foo"';
const string str_5 = "line 1
line 2
";

const list<string> escapes = [
  "\x42cafes",
  "caf\xc3\xa9",
  "\"",
  '\'',
  "\u0001",
  "\u007f",
  "\u0080",
  "\u07ff",
  "\u0800",
  "\uffff",
  "\uABCD",
  "con\
tinued",
];

const string multi_line_string = "This
is a
multi line string.
";

const list<string> l_e = [];
const list<i32> l_1 = [23, 42, 56];
const list<string> l_2 = ["foo", "bar", "baz"];

const set<string> s_e = [];
const set<i32> s_1 = [23, 42, 56];
const set<string> s_2 = ["foo", "bar", "baz"];

const map<string, string> m_e = {};
const map<i32, i32> m_1 = {23: 97, 42: 37, 56: 11};
const map<string, string> m_2 = {"foo": "bar", "baz": "gaz"};
const map<string, i32> m_3 = {'"': 34, "'": 39, "\\": 92, "\x61": 97};
const map_i32_i32_7023 m_4 = {1: 2};

struct struct1 {
  1: i32 a = 1234567;
  2: string b = "<uninitialized>";
}

const struct1 pod_0 = {};

const struct1 pod_1 = {"a": 10, "b": "foo"};

struct struct2 {
  1: i32 a;
  2: string b;
  3: struct1 c;
  4: list<i32> d;
}

const struct2 pod_2 = {
  "a": 98,
  "b": "gaz",
  "c": {"a": 12, "b": "bar"},
  "d": [11, 22, 33],
};

struct struct3 {
  1: string a;
  2: i32 b;
  3: struct2 c;
}

const struct3 pod_3 = {"a": "abc","b": 456,"c": {"a": 888, "c": {"b": "gaz"}},};

union union1 {
  1: i32 i;
  2: double d;
}

const union1 u_1_1 = {"i": 97};

const union1 u_1_2 = {"d": 5.6};

const union1 u_1_3 = {};

union union2 {
  1: i32 i;
  2: double d;
  3: struct1 s;
  4: union1 u;
}

const union2 u_2_1 = {"i": 51};

const union2 u_2_2 = {"d": 6.7};

const union2 u_2_3 = {"s": {"a": 8, "b": "abacabb"}};

const union2 u_2_4 = {"u": {"i": 43}};

const union2 u_2_5 = {"u": {"d": 9.8}};

const union2 u_2_6 = {"u": {}};

const i64 maxIntDec = 9223372036854775807;
const i64 maxIntOct = 0777777777777777777777;
const i64 maxIntHex = 0x7FFFFFFFFFFFFFFF;
const double maxDub = 1.79769313486231E308;
const double minDub = 2.2250738585072014e-308;
const double minSDub = 4.9406564584124654e-324;

const i64 maxPIntDec = +9223372036854775807;
const double maxPDub = +1.79769313486231e+308;
const double minPDub = +2.2250738585072014E-308;
const double minPSDub = +4.9406564584124654e-324;

// Causes a problematic (arguably invalid) warning in c++.
//const i64 minIntDec = -9223372036854775808;
const double maxNDub = -1.79769313486231e+308;
const double minNDub = -2.2250738585072014e-308;
const double minNSDub = -4.9406564584124654e-324;

// The following were automatically generated and may benefit from renaming.
@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<i32, i32> map_i32_i32_7023
