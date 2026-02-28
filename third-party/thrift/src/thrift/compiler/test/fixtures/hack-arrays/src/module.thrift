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

include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@hack.SkipCodegen{reason = "Invalid key type"}
typedef set<float> InvalidSetTypedef

@hack.SkipCodegen{reason = "Invalid key type"}
typedef list<InvalidSetTypedef> ListOfInvalidSetTypedef

struct Foo {
  1: list<string> a;
  2: map<string, list<set<i32>>> b;
  3: ListOfInvalidSetTypedef map_of_MyDataItem_to_MyDataItem;
}

service Bar {
  string baz(1: set<i32> a, 2: list<map<i32, set<string>>> b);
}
