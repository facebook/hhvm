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

namespace cpp2 facebook
namespace rust fbthrift_test_if

include "thrift/annotation/rust.thrift"
include "thrift/annotation/thrift.thrift"

struct Small {
  1: i32 num;
  2: i64 two;
}

struct SubStruct {
  // @lint-ignore THRIFTCHECKS
  @thrift.AllowUnsafeOptionalCustomDefaultValue
  1: optional string optDef = "IAMUSELESSOPTWITHDEF";
  2: required string req_def = "IAMREQ";
  // @lint-ignore THRIFTCHECKS
  3: optional map<Small, i32> key_map;
  4: binary bin;
}

struct UnOne {
  1: i32 one;
}
struct UnTwo {
  1: i32 two;
}
union Un {
  1: UnOne un1;
  2: UnTwo un2;
}

enum En {
  ONE = 1,
  TWO = 2,
}

struct MainStruct {
  1: string foo = "i am foo";
  2: map<string, i32> m;
  3: required string bar;
  4: SubStruct s;
  5: list<Small> l;
  6: Un u;
  7: En e;
  8: map<i32, i32> int_keys;
  9: optional string opt;
}

struct MainStructNoBinary {
  1: string foo = "i am foo";
  2: map<string, i32> m;
  3: required string bar;
  5: list<Small> l;
  6: Un u;
  7: En e;
  8: map<i32, i32> int_keys;
  9: optional string opt;
}

struct Basic {
  1: bool b;
  // skip a field id
  3: bool b2;
}

struct Containers {
  1: map<string, string> m;
  2: list<string> l;
}

struct FloatingPoint {
  1: float f32;
  2: double f64;
}

@rust.NewType
@rust.Ord
@rust.Type{name = "OrderedFloat<f64>"}
typedef double Double

safe stateful server exception TestException {
  1: string message;
}

service TestService {
  void method1(1: string req) throws (1: TestException ex);
}

safe stateful server exception TestExceptionMsgOverride {
  @thrift.ExceptionMessage
  1: string message;
}

safe stateful server exception TestExceptionMsgOverrideOptional {
  @thrift.ExceptionMessage
  1: optional string message;
}

const SubStruct DEFAULT_SUBSTRUCT = {};

const SubStruct CUSTOM_SUBSTRUCT = {
  "optDef": "CUSTOM_OPT_DEF",
  "req_def": "CUSTOM_REQ_DEF",
  "key_map": {{"num": 4, "two": 2}: 42},
  "bin": "0123456789",
};
