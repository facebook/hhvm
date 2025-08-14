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

include "thrift/annotation/cpp.thrift"

namespace py3 python_test

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf

struct Foo {
  1: i32 value;
}

struct OtherFoo {
  1: i32 value;
}

enum Bar {
  UNKNOWN = 0,
  ONE = 1,
  TWO = 2,
}

enum OtherBar {
  UNKNOWN = 0,
  ONE = 1,
  TWO = 2,
}

enum Color {
  red = 0,
  blue = 1,
  green = 2,
}

struct Lists {
  3: list<bool> boolList;
  14: list<byte> byteList;
  15: list<i16> i16List;
  9: list<i32> i32List;
  26: list<i64> i64List;
  5: list<double> doubleList;
  35: list<float> floatList;
  8: list<string> stringList;
  97: list<binary> binaryList;
  93: list<IOBuf> iobufList;
  2: list<Foo> structList;
  47: list<Color> colorList;
}

struct Sets {
  # @lint-ignore THRIFTCHECKS
  3: set<bool> boolSet;
  14: set<byte> byteSet;
  15: set<i16> i16Set;
  9: set<i32> i32Set;
  26: set<i64> i64Set;
  # @lint-ignore THRIFTCHECKS
  5: set<double> doubleSet;
  # @lint-ignore THRIFTCHECKS
  35: set<float> floatSet;
  8: set<string> stringSet;
  97: set<binary> binarySet;
  93: set<IOBuf> iobufSet;
  # @lint-ignore THRIFTCHECKS
  2: set<Foo> structSet;
  47: set<Color> colorSet;
}

struct Maps {
  # @lint-ignore THRIFTCHECKS
  3: map<bool, i32> boolMap;
  14: map<byte, byte> byteMap;
  15: map<i16, i16> i16Map;
  9: map<i32, i32> i32Map;
  26: map<i64, i64> i64Map;
  # @lint-ignore THRIFTCHECKS
  5: map<double, double> doubleMap;
  # @lint-ignore THRIFTCHECKS
  35: map<float, float> floatMap;
  8: map<string, string> stringMap;
  97: map<binary, binary> binaryMap;
  93: map<IOBuf, IOBuf> iobufMap;
  # @lint-ignore THRIFTCHECKS
  2: map<Foo, Foo> structMap;
  47: map<Color, Color> colorMap;
}

struct UnicodeContainers {
  1: list<string> stringList;
  2: set<string> stringSet;
  3: map<string, string> stringMap;
}
