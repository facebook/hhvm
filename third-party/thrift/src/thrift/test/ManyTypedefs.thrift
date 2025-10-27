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

package "thrift.org/test"
namespace go thrift.test.ManyTypedefs
namespace py3 thrift.test

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyTypedefUri
typedef list<int32> biglist
@thrift.AllowLegacyTypedefUri
@cpp.Type{template = "std::unordered_map"}
typedef map<int32, int32> bigmap
@thrift.AllowLegacyTypedefUri
typedef i32 int32

struct struct1 {
  1: int32 myint;
  2: biglist mylist;
}

exception exception1 {
  1: biglist alist;
  2: struct1 mystruct;
}

service AService {
  struct1 method1(1: int32 myint) throws (1: exception1 exn);
  biglist method2();
}
