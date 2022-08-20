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

struct ReflectionTestStruct1 {
  3: i32 c;
  1: required i32 a;
  2: optional i32 b;
  4: string d (
    some.field.annotation = "hello",
    some.other.annotation = 1,
    annotation.without.value,
  );
}

enum ReflectionTestEnum {
  FOO = 5,
  BAR = 4,
}

struct ReflectionTestStruct2 {
  1: map<byte, ReflectionTestStruct1> a;
  2: set<string> b;
  3: list<i64> c;
  4: ReflectionTestEnum d;
}
