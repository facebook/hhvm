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

include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace java.swift test.fixtures.lazy_constants
namespace hack test.fixtures.lazy_constants

const i32 myInt = 1337;
const string name = "Mark Zuckerberg";
const list<map<string, i32>> states = [
  {"San Diego": 3211000, "Sacramento": 479600, "SF": 837400},
  {"New York": 8406000, "Albany": 98400},
];
const set<string> cities = ["New York", "Sacramento"];
const double x = 1.0;
const double y = 1000000;
const double z = 1000000000.0;

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

const map<Company, i32> const_enum_map = {
  Company.FACEBOOK: 123,
  Company.WHATSAPP: 2,
};

struct Internship {
  1: required i32 weeks;
  2: string title;
  3: optional Company employer;
}

const Internship instagram = {
  "weeks": 12,
  "title": "Software Engineer",
  "employer": Company.INSTAGRAM,
};

struct Range {
  1: required i32 min;
  2: required i32 max;
}

const list<Range> kRanges = [{"min": 1, "max": 2}, {"min": 5, "max": 6}];

const list<Internship> internList = [
  instagram,
  {"weeks": 10, "title": "Sales Intern", "employer": Company.FACEBOOK},
];

const string apostrophe = "'";
const string tripleApostrophe = "'''";
const string quotationMark = '"'; //" //fix syntax highlighting
const string backslash = "\\";
const string escaped_a = "\x61";

const map<string, i32> char2ascii = {"'": 39, '"': 34, "\\": 92, "\x61": 97};
