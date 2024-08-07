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

include "thrift/annotation/python.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/any_rep.thrift"
include "thrift/lib/thrift/type.thrift"

@thrift.TerseWrite
@thrift.Experimental
package "facebook.com/thrift/op"

namespace cpp2 apache.thrift.op
namespace py3 apache.thrift.op
namespace java.swift com.facebook.thrift.op
namespace js apache.thrift.op
namespace py.asyncio apache_thrift_asyncio.any_patch
namespace go thrift.lib.thrift.any_patch
namespace py thrift.lib.thrift.any_patch

// Since Thrift discourages structured keys, we leverage 'list<TypeToPatchInternalDoNotUse>'
// to represent on wire for 'map<type.Type, list<any_rep.AnyStruct>>'.
struct TypeToPatchInternalDoNotUse {
  @python.Py3Hidden
  1: type.Type type;
  2: list<any_rep.AnyStruct> patches;
}

/** A patch for Thrift Any. */
struct AnyPatchStruct {
  /**
   * Assigns to the specified Thrift Any.
   *
   * If set, all other patch operations are ignored.
   */
  1: optional any_rep.AnyStruct assign;

  /** Clear to the intrinsic default. */
  2: bool clear;

  /**
   * Applies a patch to the stored value if its type matches the type specified in the map key.
   *
   * Invariants:
   *   - The value of the map should store valid Thrift Patch.
   *   - The key of the map should be value type for stored Thrift Patch in the map value.
   */
  10: list<TypeToPatchInternalDoNotUse> patchIfTypeIsPrior;

  /** Sets to the specified Thrift Any if the stored value's type does not match. */
  11: optional any_rep.AnyStruct ensureAny;

  /**
   * Applies a patch to the stored value if its type matches the type specified in the map key.
   *
   * Invariants:
   *   - The value of the map should store valid Thrift Patch.
   *   - The key of the map should be value type for stored Thrift Patch in the map value.
   */
  12: list<TypeToPatchInternalDoNotUse> patchIfTypeIsAfter;
}
