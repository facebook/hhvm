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

struct Pod {
  3: i32 a;
  2: i16 b;
  1: byte c;
}

struct Person {
  1: required string name;
  2: required i64 id;
  4: set<i32> nums;
  3: optional double dob;
}

struct Team {
  2: optional map<i64, Person> peopleById;
  4: optional map<i64, i64> (cpp.template = 'std::unordered_map') ssnLookup;
  3: optional map<string, Person> peopleByName;
  1: optional set<string> projects;
}
