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

package "test.dev/fixtures/java_options"

namespace java.swift test.fixtures.basic

const i64 kConstantValue = 0xBEEF;

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
  5: bool oneway;
  6: bool readonly;
  7: bool idempotent;
  8: set<float> floatSet;
  9: string no_hack_codegen_field;
}

struct MyDataItem {}

union MyUnion {
  1: MyEnum myEnum;
  2: MyStruct myStruct;
  3: MyDataItem myDataItem;
  4: set<float> floatSet;
}

service MyService {
  void ping();
  string getRandomData();
  void sink(1: i64 sink);
}

service DbMixedStackArguments {
  binary getDataByKey0(1: string key);
  binary getDataByKey1(1: string key);
}
