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

namespace java.swift test.fixtures.constants
namespace java.swift.constants test.fixtures.constants.ModuleConstants

const i32 myInt = 1337;
const string name = "Mark Zuckerberg";
const string multi_line_string = "This
is a
multi line string.
";
const list<map<string, i32>> states = [
  {"San Diego": 3211000, "Sacramento": 479600, "SF": 837400},
  {"New York": 8406000, "Albany": 98400},
];
const double x = 1.0;
const double y = 1000000;
const double z = 1000000000.0;
const double zeroDoubleValue = 0.0;

const double longDoubleValue = 0.0000259961000990301;

enum EmptyEnum {
}

enum City {
  NYC = 0,
  MPK = 1,
  SEA = 2,
  LON = 3,
}
enum Company {
  FACEBOOK = 0,
  WHATSAPP = 1,
  OCULUS = 2,
  INSTAGRAM = 3,
}

typedef Company MyCompany
const MyCompany my_company = FACEBOOK;

struct Internship {
  1: required i32 weeks;
  2: string title;
  3: optional Company employer;
  4: optional double compensation;
  5: optional string school;
}

typedef string MyStringIdentifier
typedef i32 MyIntIdentifier
typedef map<string, string> MyMapIdentifier

const MyStringIdentifier foo = "foo";
const MyIntIdentifier bar = 42;
const MyMapIdentifier mymap = {"keys": "values"};

const Internship instagram = {
  "weeks": 12,
  "title": "Software Engineer",
  "employer": Company.INSTAGRAM,
  "compensation": 1200.0,
  "school": "Monters University",
};

const Internship partial_const = {"weeks": 8, "title": "Some Job"};

struct Range {
  1: required i32 min;
  2: required i32 max;
}

const list<Range> kRanges = [{"min": 1, "max": 2}, {"min": 5, "max": 6}];

const list<Internship> internList = [
  instagram,
  {
    "weeks": 10,
    "title": "Sales Intern",
    "employer": Company.FACEBOOK,
    "compensation": 1000.0,
  },
];

struct struct1 {
  1: i32 a = 1234567;
  2: string b = "<uninitialized>";
}

const struct1 pod_0 = {};

const struct1 pod_s_0 = struct1{};

const struct1 pod_1 = {"a": 10, "b": "foo"};

const struct1 pod_s_1 = struct1{a = 10, b = foo};

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

const struct2 pod_trailing_commas = {
  "a": 98,
  "b": "gaz",
  "c": {"a": 12, "b": "bar"},
  "d": [11, 22, 33],
};

const struct2 pod_s_2 = struct2{
  a = 98,
  b = "gaz",
  c = struct1{a = 12, b = "bar"},
  d = [11, 22, 33],
};

struct struct3 {
  1: string a;
  2: i32 b;
  3: struct2 c;
}

const struct3 pod_3 = {
  "a": "abc",
  "b": 456,
  "c": {"a": 888, "c": {"b": "gaz"}, "d": [1, 2, 3]},
};

const struct3 pod_s_3 = struct3{
  a = "abc",
  b = 456,
  c = struct2{a = 888, c = struct1{b = 'gaz'}, d = [1, 2, 3]},
};

struct struct4 {
  1: i32 a;
  2: optional double b;
  3: optional byte c;
}

const struct4 pod_4 = {"a": 1234, "b": 0.333, "c": 25};

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

const string apostrophe = "'";
const string tripleApostrophe = "'''";
const string quotationMark = '"'; //" //fix syntax highlighting
const string backslash = "\\";
const string escaped_a = "\x61";

const map<string, i32> char2ascii = {"'": 39, '"': 34, "\\": 92, "\x61": 97};

const list<string> escaped_strings = [
  // \x00 - \x1f are control characters:
  "\x01",
  "\x1f",
  "\x20", // space
  "\'",
  '\"',
  "\n",
  "\r",
  "\t",
  "\x61",
  "\xc2\xab",
  "\x6a",
  "\xc2\xa6",
  "\x61yyy",
  "\xc2\xabyyy",
  "\x6ayyy",
  "\xc2\xa6yyy",
  "zzz\x61",
  "zzz\xc2\xab",
  "zzz\x6a",
  "zzz\xc2\xa6",
  "zzz\x61yyy",
  "zzz\xc2\xabyyy",
  "zzz\x6ayyy",
  "zzz\xc2\xa6yyy",
];

const bool false_c = false;
const bool true_c = true;
const byte zero_byte = 0;
const i16 zero16 = 0;
const i32 zero32 = 0;
const i64 zero64 = 0;
const double zero_dot_zero = 0.0;
const string empty_string = "";
const list<i32> empty_int_list = [];
const list<string> empty_string_list = [];
const set<i32> empty_int_set = [];
const set<string> empty_string_set = [];
const map<i32, i32> empty_int_int_map = {};
const map<i32, string> empty_int_string_map = {};
const map<string, i32> empty_string_int_map = {};
const map<string, string> empty_string_string_map = {};

const i64 maxIntDec = 9223372036854775807;
const i64 maxIntOct = 0777777777777777777777;
const i64 maxIntHex = 0x7FFFFFFFFFFFFFFF;
const i64 maxIntBin = 0b111111111111111111111111111111111111111111111111111111111111111;
const double maxDub = 1.7976931348623157e308;
const double minDub = 2.2250738585072014e-308;
const double minSDub = 4.9406564584124654e-324;

const i64 maxPIntDec = +9223372036854775807;
const i64 maxPIntOct = +0777777777777777777777;
const i64 maxPIntHex = +0X7FFFFFFFFFFFFFFF;
const i64 maxPIntBin = +0B111111111111111111111111111111111111111111111111111111111111111;
const double maxPDub = +1.7976931348623157E+308;
const double minPDub = +2.2250738585072014E-308;
const double minPSDub = +4.9406564584124654E-324;

const i64 minIntDec = -9223372036854775808;
const i64 minIntOct = -01000000000000000000000;
const i64 minIntHex = -0x8000000000000000;
const i64 minIntBin = -0b1000000000000000000000000000000000000000000000000000000000000000;
const double maxNDub = -1.7976931348623157e+308;
const double minNDub = -2.2250738585072014e-308;
const double minNSDub = -4.9406564584124654e-324;

const map<i32, bool> I2B = {
  0: false,
  1: true,
  2: true,
  3: false
};
const map<i32, bool> I2B_REF = I2B;
