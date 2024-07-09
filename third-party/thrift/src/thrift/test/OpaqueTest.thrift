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

namespace cpp2 apache.thrift.test

cpp_include "<unordered_map>"
cpp_include "thrift/test/OpaqueTest.h"

include "thrift/annotation/cpp.thrift"

@cpp.Type{name = "OpaqueDouble1"}
typedef double (cpp.indirection) OpaqueDouble1
@cpp.Type{name = "OpaqueDouble2"}
typedef double (cpp.indirection) OpaqueDouble2
@cpp.Type{name = "NonConvertibleId"}
typedef i64 (cpp.indirection) NonConvertibleId
@cpp.Type{template = "std::unordered_map"}
typedef map<i32, OpaqueDouble1> OpaqueValuedMap

struct OpaqueTestStruct {
  1: OpaqueDouble1 d1;
  2: OpaqueDouble2 d2;
  3: OpaqueValuedMap dmap;
  4: list<NonConvertibleId> ids;
}
