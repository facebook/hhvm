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

include "thrift/annotation/python.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace py3 python_test

@python.Adapter{
  name = "thrift.python.test.adapters.atoi.AtoiAdapter",
  typeHint = "int",
}
typedef string AtoIValue

typedef list<i32> I32List
typedef set<easy> EasySet
typedef set<i32> SetI32
typedef set<AtoIValue> SetAtoIValue
typedef set<I32List> SetI32Lists
typedef set<SetI32Lists> SetSetI32Lists

const set<string> constant_set = ["1", "2", "3"];

struct easy {
  3: optional string name;
  1: i32 val;
  2: I32List val_list;
  @python.Py3Hidden{}
  4: i64 py3_hidden;
}
