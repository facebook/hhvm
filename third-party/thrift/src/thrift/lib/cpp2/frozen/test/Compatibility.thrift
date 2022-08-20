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

namespace cpp2 apache.thrift.test

struct Person {
  3: optional double dob;
  5: string name;
}

struct Root {
  1: string title;
  2: map<i64, Person> (cpp.template = 'std::unordered_map') people;
}

struct Case {
  1: string name;
  2: optional Root root;
  3: bool fails = 0;
}

const list<Case> kTestCases = [
  {
    "name": "beforeUnique",
    "root": {
      "title": "version 0",
      "people": {
        101: {"dob": 1.23e9, "name": "alice"},
        102: {"dob": 1.21e9, "name": "bob"},
      },
    },
  },
  {
    "name": "afterUnique",
    "root": {
      "title": "version 0",
      "people": {
        101: {"dob": 1.23e9, "name": "alice"},
        102: {"dob": 1.21e9, "name": "bob"},
      },
    },
  },
  {
    "name": "withFileVersion",
    "root": {
      "title": "version 0",
      "people": {
        101: {"dob": 1.23e9, "name": "alice"},
        102: {"dob": 1.21e9, "name": "bob"},
      },
    },
  },
  {"name": "futureVersion", "fails": 1},
  {"name": "missing", "fails": 1},
];
