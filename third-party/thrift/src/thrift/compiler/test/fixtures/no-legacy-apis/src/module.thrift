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

include "thrift/annotation/thrift.thrift"

@thrift.NoLegacy
package "test.dev/fixtures/no-legacy-apis"

namespace cpp2 test.fixtures.basic
namespace py3 test.fixtures.basic
namespace android test.fixtures.basic
namespace java test.fixtures.basic
namespace java.swift test.fixtures.basic

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
}

struct MyStruct {
  1: i64 myIntField;
  2: string myStringField;
}

union MyUnion {
  1: MyEnum myEnum;
  2: MyStruct myDataItem;
}

service MyService {
  MyStruct query(1: MyUnion u);
}
