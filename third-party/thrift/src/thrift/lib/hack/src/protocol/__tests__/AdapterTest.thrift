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

package "meta.com/thrift/adapter_test"

namespace hack AdapterTest
include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\AdapterTestReverseList'}
typedef list<i32> ReversedList

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\AdapterTestJsonToShape'}
typedef string StringWithAdapterTestJsonToShape

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\AdapterTestStructToShape'}
typedef Bar BarWithAdapter

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\AdapterTestIntToString'}
typedef i32 i32WithAdapter

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\AdapterTestThrowingAdapter'}
typedef i32 i32WithThrowingAdapter

struct Foo {
  1: i32WithAdapter intField;
  @hack.Adapter{name = '\AdapterTestIntToString'}
  2: optional i32 oIntField;
  3: list<StringWithAdapterTestJsonToShape> listField;
  4: optional list<StringWithAdapterTestJsonToShape> oListField;
  5: BarWithAdapter structField;
  6: optional BarWithAdapter oStructField;
  7: ReversedList reversedListField;
  8: map<string, BarWithAdapter> mapField;
}

struct StructWithThrowingAdapter {
  1: i32WithThrowingAdapter field;
}

struct FooWithoutAdapters {
  1: i32 intField;
  2: optional i32 oIntField;
  3: list<string> listField;
  4: optional list<string> oListField;
  5: Bar structField;
  6: optional Bar oStructField;
  7: list<i32> reversedListField;
  8: map<string, Bar> mapField;
}

struct Bar {
  1: i32 field;
}

service Service {
  ReversedList func(1: i32WithAdapter arg1, 2: Foo arg2);
}
