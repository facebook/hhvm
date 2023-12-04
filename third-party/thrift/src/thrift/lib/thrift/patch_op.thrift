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

package "facebook.com/thrift/op"

namespace cpp2 apache.thrift.op
namespace py3 apache.thrift.op
namespace java com.facebook.thrift.op
namespace java.swift com.facebook.thrift.op
namespace js apache.thrift.op
namespace py.asyncio apache_thrift_asyncio.patch_op
namespace go thrift.lib.thrift.patch_op
namespace py thrift.lib.thrift.patch_op

/**
 * The meaning of the patch op field ids, in all properly formulated patch
 * definitions.
 *
 * Patch field ids are interpreted at runtime, as a dynamic patch protocol,
 * without any additional schema derived from IDL patch definitions.
 */
enum PatchOp {
  Unspecified = 0,

  /**
   * Set the value. Supersedes all other ops.
   *
   * Note: Due to a limitation in current Thrift protocol encoding schemes,
   * unions cannot be reliabily distinquished from structs/exceptions, so fields
   * must be set before assign can change the value, for example via an
   * `EnsureStruct` or `EnsureUnion` operation.
   */
  Assign = 1,

  /** Set to the intrinsic default (which might be 'unset'). */
  Clear = 2,

  /** Apply a field/value-wise patch. */
  PatchPrior = 3,

  /**
   * Set to the given default, if not already of the same type.
   *
   * In a dynamic context this means the ids/values must match exactly:
   *     ensureUnion(Object ensureUnion, Object value) {
   *       if (ensureUnion.ids() != value.ids())
   *         value = ensureUnion;
   *     }
   */
  EnsureUnion = 4,

  /**
   * A pair-wise ensure operation.
   *
   * For maps this is an "add if key not present".
   *
   * For structs, this can be use to encodes the default state of the fields, based
   * on thier qualifier type:
   * - optional: absent
   * - terse: intrinsic default
   * - fill: custom default
  **/
  EnsureStruct = 5,

  // TODO(afuller): Add a variant of ensure, which only ensures if 'unset'.

  /** Apply a field/value-wise patch after all other ops. */
  PatchAfter = 6,

  /**
   * Remove if present.
   *
   * A key/value-based remove for set, 'saturating subtract' for
   * numeric/'counting' types, 'remove by key' for map, and `remove by field id` for struct.
   */
  Remove = 7,

  /**
   * Add/prepend a value,with the following semantics:
   * - Key/value-based 'add' for set;
   * - 'prepend' for list, string, or binary; and
   * - saturating 'add' for numeric/counting types.
   */
  Add = 8,

  /**
   * Put/append/invert a value, with the following semantics:
   * - 'update or insert' for maps;
   * - 'append' for list, string or binary; and
   * - 'invert' for boolean.
   */
  Put = 9,
}
