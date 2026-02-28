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

include "thrift/annotation/python.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 convertible

enum Empty {
}

enum Color {
  NONE = 0,
  RED = 1,
  GREEN = 2,
  BLUE = 3,
}

enum Shade {
  NONE = 0,
  CRIMSON = 1,
  EMERALD = 2,
  AZURE = 3,
}

struct Simple {
  1: i32 intField;
  2: string strField;
  3: list<i32> intList;
  10: list<string> strList;
  4: set<string> strSet;
  5: map<string, i64> strToIntMap;
  9: map<string, string> strToStrMap;
  6: Color color;
  @python.Name{name = "name_"}
  7: string name;
  8: Empty empty;
}

struct Nested {
  1: Simple simpleField;
  2: list<Simple> simpleList;
  3: map<Color, Simple> colorToSimpleMap;
  4: optional Simple optionalSimple;
}

union Union {
  1: i32 intField;
  2: string strField;
  3: list<i32> intList;
  @python.Name{name = "simple_"}
  4: Simple simpleField;
  @python.Name{name = "name_"}
  5: string name;
}

struct OptionalDefaultsStruct {
  // @lint-ignore THRIFTCHECKS
  @thrift.AllowUnsafeOptionalCustomDefaultValue
  1: optional string sillyString = "default string";
  // @lint-ignore THRIFTCHECKS
  @thrift.AllowUnsafeOptionalCustomDefaultValue
  2: optional Color sillyColor = Color.RED;
}

struct Tomayto {
  1: i64 to;
  2: string mayto = "mayto";
}

struct Tomahto {
  1: i64 to;
  3: string mahto = "mahto";
}

union Potayto {
  1: i64 po;
  2: string tay;
  4: bool to;
}

union Potahto {
  1: i64 po;
  3: string tah;
  4: bool to;
}

exception SimpleException {
  1: string message;
  2: i32 code;
}
