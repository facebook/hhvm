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

package "thrift.com/python/test"

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/python.thrift"

namespace py3 python_test

@python.Adapter{
  name = "thrift.python.test.adapters.atoi.AtoiAdapter",
  typeHint = "int",
}
typedef string AtoIValue

const map<i16, map<i16, i16>> LocationMap = {1: {1: 1}};
typedef list<i32> I32List
typedef map<string, i64> StrIntMap
typedef map<string, I32List> StrI32ListMap
typedef map<string, easy> StrEasyMap
typedef map<string, string> StrStrMap
typedef map<string, AtoIValue> StrAtoIValueMap
typedef map<string, StrI32ListMap> StrStrIntListMapMap
@cpp.Type{name = "folly::F14FastMap<std::string, folly::fbstring>"}
typedef map<string, string> F14MapFollyString

const map<string, i32> constant_map = {"1": 1, "2": 2, "3": 3};

struct easy {
  3: optional string name;
  1: i32 val;
  2: I32List val_list;
  @python.Py3Hidden{}
  4: i64 py3_hidden;
}
