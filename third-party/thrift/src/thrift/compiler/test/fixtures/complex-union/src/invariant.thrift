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

namespace java.swift test.fixtures.complex_union

struct StructForInvariantTypes {
  1: i64 num;
}

struct UnionForInvariantTypes {
  1: i32 num32;
  2: i64 num64;
}

union InvariantTypes {
  1: map<StructForInvariantTypes, i64> struct_map;
  2: map<UnionForInvariantTypes, i64> union_map;
}
