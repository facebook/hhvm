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

package "facebook.com/thrift/test/frozen/adapter"

include "thrift/annotation/cpp.thrift"

cpp_include "thrift/lib/cpp2/frozen/test/FrozenAdapterTestUtil.h"

// Field-level @cpp.Adapter storage types name their enclosing struct, so the
// layouts header (emitted under namespace apache::thrift::frozen) can only
// compile these structs if that name is fully qualified.
struct AdaptedNested {
  @cpp.Adapter{name = "::apache::thrift::test::PackedIntListAdapter"}
  1: binary ids;
}

struct AdaptedFields {
  @cpp.Adapter{name = "::apache::thrift::test::PackedIntListAdapter"}
  1: binary ids;

  @cpp.Adapter{name = "::apache::thrift::test::WidenToI64FieldAdapter"}
  2: i32 widened;

  @cpp.Adapter{name = "::apache::thrift::test::PackedIntListAdapter"}
  3: optional binary optionalIds;

  4: AdaptedNested nested;

  5: list<AdaptedNested> nestedList;

  6: string plain;
}
