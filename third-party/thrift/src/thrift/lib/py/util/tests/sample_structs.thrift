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

@thrift.AllowLegacyMissingUris
package;

namespace py3 thrift.test
namespace py thrift.test.sample_structs

struct DummyStruct {
  1: i32 a;
}

struct Struct {
  1: i32 a;
  2: list<i32> b;
  3: set<i32> c;
  4: map<i32, i32> d;
  5: map<i32, list<i32>> e;
  6: map<i32, set<i32>> f;
  7: list<DummyStruct> g;
  8: list<list<i32>> h;
  9: list<set<i32>> i;
}
