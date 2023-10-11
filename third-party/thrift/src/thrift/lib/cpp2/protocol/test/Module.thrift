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

package "apache.org/thrift/test"

cpp_include "<unordered_set>"
cpp_include "<unordered_map>"

cpp_include "folly/container/F14Map.h"
cpp_include "folly/container/F14Set.h"

include "thrift/annotation/cpp.thrift"

struct StructWithEmptyMap {
  1: map<string, i64> myMap;
}

struct SubStruct {
  3: i64 mySubI64 = 17;
  12: string mySubString = "foobar";
}

union SubUnion {
  209: string text;
}

const SubUnion kSubUnion = {"text": "glorious"};

struct OneOfEach {
  1: bool myBool = 1;
  2: byte myByte = 17;
  3: i16 myI16 = 1017;
  4: i32 myI32 = 100017;
  5: i64 myI64 = 5000000017;
  6: double myDouble = 5.25;
  7: float myFloat = 5.25;
  8: map<string, i64> myMap = {"foo": 13, "bar": 17, "baz": 19};
  9: list<string> myList = ["foo", "bar", "baz"];
  10: set<string> mySet = ["foo", "bar", "baz"];
  11: SubStruct myStruct;
  12: SubUnion myUnion = kSubUnion;
}

struct OneOfEach2 {
  1: bool myBool = 1;
  2: byte myByte = 17;
  3: i16 myI16 = 1017;
  4: i32 myI32 = 100017;
  5: i64 myI64 = 5000000017;
  6: double myDouble = 5.25;
  7: float myFloat = 5.25;
  8: map<i32, i64> myMap;
  9: list<string> myList = ["foo", "bar", "baz"];
  10: set<string> mySet = ["foo", "bar", "baz"];
  11: SubStruct myStruct;
  12: SubUnion myUnion = kSubUnion;
}

struct OneOfEach3 {
  1: bool myBool = 1;
  2: byte myByte = 17;
  3: i16 myI16 = 1017;
  4: i32 myI32 = 100017;
  5: i64 myI64 = 5000000017;
  6: double myDouble = 5.25;
  7: float myFloat = 5.25;
  8: map<string, i64> myMap = {"foo": 13, "bar": 17, "baz": 19};
  9: list<double> myList;
  10: set<list<i32>> mySet;
  11: SubStruct myStruct;
  12: SubUnion myUnion = kSubUnion;
}

struct DebugHashedAssociative {
  @cpp.Type{name = "std::map<int64_t, std::set<int64_t>>"}
  1: map<i64, set<i64>> value;
}

@cpp.Type{template = "folly::F14FastSet"}
typedef set<i64> F14SetI64

struct DebugSortedAssociative {
  @cpp.Type{template = "folly::F14FastMap"}
  1: map<i64, F14SetI64> value;
}

struct DebugList {
  1: list<i32> aList;
}

struct StructWithF14VectorContainers {
  @cpp.Type{template = "folly::F14VectorMap"}
  1: map<i32, i32> m;
  @cpp.Type{template = "folly::F14VectorSet"}
  2: set<i32> s;
}

struct OrderedFields {
  1: map<string, i64> f1 = {"a": 1, "b": 2, "c": 3};
  3: string f3 = "d";
  2: set<string> f2 = ["a", "b", "c"];
}

struct UnorderedFields {
  @cpp.Type{template = "std::unordered_set"}
  2: set<string> f2 = ["a", "b", "c"];
  @cpp.Type{template = "std::unordered_map"}
  1: map<string, i64> f1 = {"a": 1, "b": 2, "c": 3};
  3: string f3 = "d";
}

struct CppTemplateListField {
  @cpp.Type{template = "std::deque"}
  1: list<string> f1 = ["1", "2", "3"];
}

struct OptionalFields {
  1: optional string f1;
  2: optional set<string> f2;
  3: optional list<string> f3;
}

struct NonStringMapKeyFields {
  // @lint-ignore THRIFTCHECKS
  1: map<bool, i32> f1;
  2: map<byte, i32> f2;
  3: map<i16, i32> f3;
  4: map<i32, i32> f4;
  5: map<i64, i32> f5;
  // @lint-ignore THRIFTCHECKS
  6: map<float, i32> f6;
  // @lint-ignore THRIFTCHECKS
  7: map<double, i32> f7;
}

union UnionIntegers {
  1: i16 myI16;
  2: i32 myI32;
}

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf

// We need (cpp.noncomparable) since the codegen version of comparison operators
// won't work
struct ListIOBuf {
  1: list<IOBuf> field;
  @cpp.Type{template = "folly::F14FastMap"}
  2: map<string, IOBuf> field_2;
} (cpp.noncomparable)

struct StructWithBinaryField {
  1: binary field;
}
