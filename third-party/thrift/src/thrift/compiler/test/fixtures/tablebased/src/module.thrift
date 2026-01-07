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

namespace cpp test.fixtures.tablebased
namespace cpp2 test.fixtures.tablebased

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr

enum ExampleEnum {
  ZERO = 0,
  NONZERO = 123,
}

struct TrivialTypesStruct {
  1: optional i32 fieldA;
  2: optional string fieldB;
  3: optional binary fieldC;
  4: optional IOBufPtr fieldD;
  5: ExampleEnum fieldE;
}

struct ContainerStruct {
  12: list<i32> fieldA;
  @cpp.Type{template = "std::list"}
  2: list<i32> fieldB;
  @cpp.Type{template = "std::deque"}
  3: list<i32> fieldC;
  @cpp.Type{template = "folly::fbvector"}
  4: list<i32> fieldD;
  @cpp.Type{template = "folly::small_vector"}
  5: list<i32> fieldE;
  @cpp.Type{template = "folly::sorted_vector_set"}
  6: set<i32> fieldF;
  @cpp.Type{template = "folly::sorted_vector_map"}
  7: map<i32, string> fieldG;
  8: list<TrivialTypesStruct> fieldH;
}

union ExampleUnion {
  1: ContainerStruct fieldA;
  2: TrivialTypesStruct fieldB;
}
