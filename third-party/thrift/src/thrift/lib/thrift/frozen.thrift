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

cpp_include "folly/sorted_vector_types.h"

namespace cpp2 apache.thrift.frozen.schema

include "thrift/annotation/cpp.thrift"

struct Field {
  // layout id, indexes into layouts
  1: i16 layoutId;
  // field offset:
  //  < 0: -(bit offset)
  //  >= 0: byte offset
  2: i16 offset = 0;
}

struct Layout {
  1: i32 size = 0;
  2: i16 bits = 0;
  @cpp.Type{template = "folly::sorted_vector_map"}
  3: map<i16, Field> fields;
  4: string typeName;
}

const i32 kCurrentFrozenFileVersion = 1;

struct Schema {
  // File format version, incremented on breaking changes to Frozen2
  // implementation.  Only backwards-compatibility is guaranteed.
  4: i32 fileVersion = 0;
  // Field type names may not change unless relaxTypeChecks is set.
  1: bool relaxTypeChecks = 0;
  @cpp.Type{template = "folly::sorted_vector_map"}
  2: map<i16, Layout> layouts;
  3: i16 rootLayout = 0;
}
