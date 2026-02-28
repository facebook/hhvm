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
 *
 */

include "thrift/lib/thrift/any.thrift"
include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test"

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = "\ThriftLazyAnyAdapter"}
typedef any.Any ThriftLazyAny

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = "\ThriftLazyAnySimpleJsonAdapter"}
typedef any.Any ThriftLazyAnySimpleJson

enum ExampleEnum {
  ENUM_VALUE_0 = 0,
  ENUM_VALUE_1 = 1,
}

struct MainStruct {
  1: ThriftLazyAny field;
}

struct ExampleStruct {
  1: i32 num;
  3: list<string> vec;
}

struct DifferentStruct {
  1: i32 num;
  2: string whatever;
}

struct AnyTestHelper {
  1: any.Any field;
}

struct OptionalStruct {
  1: optional ThriftLazyAny optional_field;
}

struct MainStructSimpleJson {
  1: ThriftLazyAnySimpleJson field;
}

struct HashsetStruct {
  1: set<i32> hashSet;
}
