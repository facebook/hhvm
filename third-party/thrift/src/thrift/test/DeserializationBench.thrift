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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct StructA {
  1: bool fieldA;
  2: i32 fieldB;
  3: string fieldC;
  4: list<i32> fieldD;
  5: set<i32> fieldE;
  6: map<i32, string> fieldF;
  7: list<list<list<i32>>> fieldG;
  8: set<set<set<i32>>> fieldH;
  9: map<map<i32, string>, map<i32, string>> fieldI;
  10: map<list<set<i32>>, set<list<i32>>> fieldJ;
}

@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<i32, string> folly_map
@cpp.Type{template = "folly::sorted_vector_set"}
typedef set<i32> folly_set
@cpp.Type{template = "folly::sorted_vector_set"}
typedef set<folly_set> folly_set_set
@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<folly_map, folly_map> folly_map_map
@cpp.Type{template = "folly::sorted_vector_set"}
typedef set<list<i32>> folly_list_set
@cpp.Type{template = "folly::sorted_vector_map"}
typedef map<list<folly_set>, folly_list_set> folly_list_set_map

struct StructB {
  1: bool fieldA;
  2: i32 fieldB;
  3: string fieldC;
  4: list<i32> fieldD;
  5: folly_set fieldE;
  6: folly_map fieldF;
  7: list<list<list<i32>>> fieldG;
  8: set_folly_set_set_4532 fieldH;
  9: folly_map_map fieldI;
  10: folly_list_set_map fieldJ;
}

// The following were automatically generated and may benefit from renaming.
@cpp.Type{template = "folly::sorted_vector_set"}
typedef set<folly_set_set> set_folly_set_set_4532
