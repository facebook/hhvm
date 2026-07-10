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

include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace hack test.fixtures.hack_attributes

// Test @hack.UnsafeArray on fields (requires non-collections mode for vec/dict)
struct UnsafeArrayFields {
  @hack.UnsafeArray
  1: list<i64> unsafe_list;
  @hack.UnsafeArray
  2: map<string, i64> unsafe_map;
  3: list<string> normal_list;
}

// Test both @hack.FixmeWrongType and @hack.UnsafeArray on same field
struct CombinedAnnotations {
  @hack.FixmeWrongType
  @hack.UnsafeArray
  1: list<i64> fixme_unsafe_list;
  @hack.UnsafeArray
  2: map<string, string> unsafe_map_only;
}
