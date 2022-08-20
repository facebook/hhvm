/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

struct Struct {
  1: string (cpp.type = "TestFileBackedString") str;
  2: binary (cpp.type = "folly::IOBuf") iobuf;
  3: optional binary (cpp.type = "std::unique_ptr<folly::IOBuf>") unique_iobuf;
  4: list<i64> (cpp.type = "MockSize<std::vector<std::int64_t>>") l;
  5: set<i64> (cpp.type = "MockSize<std::set<std::int64_t>>") s;
  6: map<i64, i64> (
    cpp.type = "MockSize<std::map<std::int64_t, std::int64_t>>",
  ) m;
}
