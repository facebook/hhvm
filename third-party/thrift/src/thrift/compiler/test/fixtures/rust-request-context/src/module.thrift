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

namespace android test.fixtures.basic
namespace java test.fixtures.basic
namespace java.swift test.fixtures.basic

include "thrift/annotation/rust.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
}

struct MyStruct {
  1: i64 MyIntField;
  2: string MyStringField;
  # use the type before it is defined. Thrift should be able to handle this
  3: MyDataItem MyDataField;
  4: MyEnum myEnum;
}

struct MyDataItem {}

union MyUnion {
  1: MyEnum myEnum;
  2: MyStruct myStruct;
  3: MyDataItem myDataItem;
}

exception MyException {}

interaction MyInteraction {
  void ping();
}

@rust.RequestContext
service MyService {
  void ping();
  string getRandomData();
  bool hasDataById(1: i64 id);
  string getDataById(1: i64 id);
  void putDataById(1: i64 id, 2: string data);
  oneway void lobDataById(1: i64 id, 2: string data);
  stream<MyStruct> streamById(1: i64 id);
  stream<MyStruct throws (1: MyException e)> streamByIdWithException(1: i64 id);
  MyDataItem, stream<MyStruct> streamByIdWithResponse(1: i64 id);
  performs MyInteraction;
  MyInteraction startPingInteraction();
}
