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

package "test.dev/fixtures/basic"

namespace android test.fixtures.basic
namespace java test.fixtures.basic
namespace java.swift test.fixtures.basic
namespace hack.module hack.module.test

include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

const bool FLAG = true;
const byte OFFSET = -10; // byte is an 8-bit signed integer
const i16 COUNT = 200;
const i32 MASK = 0xFA12EE;
const double E = 2.718281828459;
const string DATE = "June 28, 2017";

const list<i32> AList = [2, 3, 5, 7];

const set<string> ASet = ["foo", "bar", "baz"];

const map<string, list<i32>> AMap = {"foo": [1, 2, 3, 4], "bar": [10, 32, 54]};

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
}

@thrift.AllowLegacyTypedefUri
typedef MyEnum MyEnumAlias

struct MyStruct {
  1: i64 MyIntField;
  2: string MyStringField;
  # use the type before it is defined. Thrift should be able to handle this
  3: MyDataItemAlias MyDataField;
  4: MyEnum myEnum;
  5: bool oneway;
  6: bool readonly;
  7: bool idempotent;
  @hack.SkipCodegen{reason = "Invalid key type"}
  8: set<float> floatSet;
  @hack.SkipCodegen{reason = "skip field codegen for deprecation"}
  9: string no_hack_codegen_field;
}

struct Containers {
  1: list<i32> I32List;
  2: set<string> StringSet;
  3: map<string, i64> StringToI64Map;
}

struct MyDataItem {}
@thrift.AllowLegacyTypedefUri
typedef MyDataItem MyDataItemAlias

union MyUnion {
  1: MyEnumAlias myEnum;
  2: MyStruct myStruct;
  3: MyDataItem myDataItem;
  @hack.SkipCodegen{reason = "Invalid key type"}
  4: set<float> floatSet;
}

exception MyException {
  1: i64 MyIntField;
  2: string MyStringField;
  3: MyStruct myStruct;
  4: MyUnion myUnion;
}

exception MyExceptionWithMessage {
  1: i64 MyIntField;
  @thrift.ExceptionMessage
  2: string MyStringField;
  3: MyStruct myStruct;
  4: MyUnion myUnion;
}

@hack.Name{name = "MyRenamedStruct"}
struct ReservedKeyword {
  @hack.Name{name = "renamed_field"}
  1: i32 reserved_field;
}

@hack.Name{name = "RenamedEnum"}
enum HackEnum {
  Value1 = 0,
  @hack.Name{name = "renamedValue"}
  Value2 = 1,
}

@hack.Name{name = "MyRenamedUnion"}
union UnionToBeRenamed {
  @hack.Name{name = "renamed_field"}
  1: i32 reserved_field;
}

@hack.Name{name = "RenamedService"}
service FooService {
  void simple_rpc();
}

service FB303Service {
  @hack.Name{name = "renamed_rpc"}
  ReservedKeyword simple_rpc(
    @hack.Name{name = "renamed_parameter"}
    1: i32 int_parameter,
  );
}

service MyService {
  void ping();
  string getRandomData();
  void sink(1: i64 sink);

  void putDataById(1: i64 id, 2: string data);
  readonly bool hasDataById(1: i64 id);
  readonly string getDataById(1: i64 id);
  idempotent void deleteDataById(1: i64 id);
  oneway void lobDataById(1: i64 id, 2: string data);

  @hack.SkipCodegen{reason = "Invalid key type"}
  set<float> invalid_return_for_hack();

  @hack.SkipCodegen{reason = "Skip function deprecation"}
  void rpc_skipped_codegen();
}

service DbMixedStackArguments {
  binary getDataByKey0(1: string key);
  binary getDataByKey1(1: string key);
}
