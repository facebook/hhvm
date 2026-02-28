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

cpp_include "folly/io/IOBuf.h"
cpp_include "thrift/test/ContainerSize.h"

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct Struct {
  @cpp.Type{name = "TestFileBackedString"}
  1: string str;
  @cpp.Type{name = "folly::IOBuf"}
  2: binary iobuf;
  @cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
  3: optional binary unique_iobuf;
  @cpp.Type{name = "MockSize<std::list<std::int64_t>>"}
  4: list<i64> l;
  @cpp.Type{name = "MockSize<std::set<std::int64_t>>"}
  5: set<i64> s;
  @cpp.Type{name = "MockSize<std::map<std::int64_t, std::int64_t>>"}
  6: map<i64, i64> m;
}
