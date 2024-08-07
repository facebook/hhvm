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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/python.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/any_rep.thrift"
include "thrift/lib/thrift/any_patch_detail.thrift"

cpp_include "thrift/lib/cpp2/op/detail/Patch.h"
cpp_include "thrift/lib/cpp2/op/detail/AnyPatch.h"

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

/** A patch for Thrift Any. */
@cpp.Adapter{
  underlyingName = "AnyPatchStruct",
  name = "::apache::thrift::op::detail::AnyPatchAdapter<::apache::thrift::op::AnyPatchStruct>",
}
struct AnyPatch {
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
  @cpp.Adapter{name = "::apache::thrift::op::detail::TypeToPatchMapAdapter"}
  @python.Py3Hidden
  10: list<any_patch_detail.TypeToPatchInternalDoNotUse> patchIfTypeIsPrior;

  /** Sets to the specified Thrift Any if the stored value's type does not match. */
  11: optional any_rep.AnyStruct ensureAny;

  /**
   * Applies a patch to the stored value if its type matches the type specified in the map key.
   *
   * Invariants:
   *   - The value of the map should store valid Thrift Patch.
   *   - The key of the map should be value type for stored Thrift Patch in the map value.
   */
  @cpp.Adapter{name = "::apache::thrift::op::detail::TypeToPatchMapAdapter"}
  @python.Py3Hidden
  12: list<any_patch_detail.TypeToPatchInternalDoNotUse> patchIfTypeIsAfter;
}
