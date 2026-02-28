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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

cpp_include "<list>"
cpp_include "folly/small_vector.h"
cpp_include "thrift/test/AdapterTest.h"

struct IncompleteMap {
  1: optional map<i32, IncompleteMapDep> field;
}
struct IncompleteMapDep {}

struct CompleteMap {
  1: optional map_i32_CompleteMapDep_4031 field;
}
struct CompleteMapDep {}

struct IncompleteList {
  1: optional list_IncompleteListDep_5764 field;
}
struct IncompleteListDep {}

struct CompleteList {
  1: optional list_CompleteListDep_2043 field;
}
struct CompleteListDep {}

struct AdaptedList {
  1: optional list<AdaptedListDep> field;
}
@cpp.Adapter{
  name = "IdentityAdapter<detail::AdaptedListDep>",
  adaptedType = "detail::AdaptedListDep",
}
struct AdaptedListDep {
  1: AdaptedList field;
}

struct DependentAdaptedList {
  1: optional list<DependentAdaptedListDep> field;
}
@cpp.Adapter{name = "IdentityAdapter<detail::DependentAdaptedListDep>"}
struct DependentAdaptedListDep {
  @thrift.Box
  1: optional i16 field;
}

// The following were automatically generated and may benefit from renaming.
@cpp.Type{template = "folly::small_vector"}
typedef list<CompleteListDep> list_CompleteListDep_2043
@cpp.Type{template = "::std::list"}
typedef list<IncompleteListDep> list_IncompleteListDep_5764
@cpp.Type{template = "std::unordered_map"}
typedef map<i32, CompleteMapDep> map_i32_CompleteMapDep_4031
