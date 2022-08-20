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

namespace cpp2 apache.thrift

struct struct0 {}

union union0 {}

exception exception0 {}

enum enum0 {
}

struct struct1 {
  1: string field1;
}

union union1 {
  1: string field1;
}

exception exception1 {
  1: string field1;
}

enum enum1 {
  field1 = 1,
}

struct struct2 {
  1: struct1 field1;
  2: union1 field2;
  3: exception1 field3;
  4: enum1 field4;
}

union union2 {
  1: struct1 field1;
  2: union1 field2;
  3: exception1 field3;
  4: enum1 field4;
}

exception exception2 {
  1: struct1 field1;
  2: union1 field2;
  3: exception1 field3;
  4: enum1 field4;
}

enum enum2 {
  field1 = 1,
  field2 = 2,
  field3 = 3,
  field4 = 4,
}
