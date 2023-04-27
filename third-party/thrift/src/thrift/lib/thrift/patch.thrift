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
include "thrift/annotation/scope.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/standard.thrift"

cpp_include "thrift/lib/cpp2/op/detail/Patch.h"

// TODO(dokwon): Remove @thrift.TerseWrite after fixing @scope.Transitive bug.
@thrift.TerseWrite
@thrift.Experimental
@thrift.v1alpha
package "facebook.com/thrift/op"

namespace cpp2 apache.thrift.op
namespace py3 apache.thrift.op
namespace java com.facebook.thrift.op
namespace java.swift com.facebook.thrift.op
namespace js apache.thrift.op
namespace py.asyncio apache_thrift_asyncio.patch
namespace go thrift.lib.thrift.patch
namespace py thrift.lib.thrift.patch

/**
 * An annotation that indicates a patch representation
 * should be generated for the associated definition.
 */
@scope.Program
@scope.Structured
struct GeneratePatch {}

@scope.Field
@scope.Structured
struct AssignOnlyPatch {}

/** A patch for a boolean value. */
@cpp.Adapter{
  underlyingName = "BoolPatchStruct",
  name = "::apache::thrift::op::detail::BoolPatchAdapter<::apache::thrift::op::BoolPatchStruct>",
}
struct BoolPatch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional bool assign;

  /** Clear any set value. */
  2: bool clear;

  /** If the bool value should be inverted. */
  9: bool invert;
}

/** A patch for an 8-bit integer value. */
@cpp.Adapter{
  underlyingName = "BytePatchStruct",
  name = "::apache::thrift::op::detail::NumberPatchAdapter<::apache::thrift::op::BytePatchStruct>",
}
struct BytePatch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional byte assign;

  /** Clear any set value. */
  2: bool clear;

  /** Add to a given value. */
  8: byte add;
}

/** A patch for a 16-bit integer value. */
@cpp.Adapter{
  underlyingName = "I16PatchStruct",
  name = "::apache::thrift::op::detail::NumberPatchAdapter<::apache::thrift::op::I16PatchStruct>",
}
struct I16Patch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional i16 assign;

  /** Clear any set value. */
  2: bool clear;

  /** Add to a given value. */
  8: i16 add;
}

/** A patch for a 32-bit integer value. */
@cpp.Adapter{
  underlyingName = "I32PatchStruct",
  name = "::apache::thrift::op::detail::NumberPatchAdapter<::apache::thrift::op::I32PatchStruct>",
}
struct I32Patch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional i32 assign;

  /** Clears any set value. */
  2: bool clear;

  /** Add to a given value. */
  8: i32 add;
}

/** A patch for a 64-bit integer value. */
@cpp.Adapter{
  underlyingName = "I64PatchStruct",
  name = "::apache::thrift::op::detail::NumberPatchAdapter<::apache::thrift::op::I64PatchStruct>",
}
struct I64Patch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional i64 assign;

  /** Clear any set value. */
  2: bool clear;

  /** Add to a given value. */
  8: i64 add;
}

/** A patch for a 32-bit floating point value. */
@cpp.Adapter{
  underlyingName = "FloatPatchStruct",
  name = "::apache::thrift::op::detail::NumberPatchAdapter<::apache::thrift::op::FloatPatchStruct>",
}
struct FloatPatch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional float assign;

  /** Clear any set value. */
  2: bool clear;

  /** Add to a given value. */
  8: float add;
}

/** A patch for an 64-bit floating point value. */
@cpp.Adapter{
  underlyingName = "DoublePatchStruct",
  name = "::apache::thrift::op::detail::NumberPatchAdapter<::apache::thrift::op::DoublePatchStruct>",
}
struct DoublePatch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional double assign;

  /** Clear any set value. */
  2: bool clear;

  /** Add to a given value. */
  8: double add;
}

/** A patch for a string value. */
@cpp.Adapter{
  underlyingName = "StringPatchStruct",
  name = "::apache::thrift::op::detail::StringPatchAdapter<::apache::thrift::op::StringPatchStruct>",
}
struct StringPatch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional string assign;

  /** Clear a given string. */
  2: bool clear;

  /** Prepend to a given value. */
  8: string prepend;

  /** Append to a given value. */
  9: string append;
}

/** A patch for a binary value. */
@cpp.Adapter{
  underlyingName = "BinaryPatchStruct",
  name = "::apache::thrift::op::detail::BinaryPatchAdapter<::apache::thrift::op::BinaryPatchStruct>",
}
struct BinaryPatch {
  /**
   * Assigns to a (set) value.
   *
   * If set, all other patch operations are ignored.
   *
   * Note: Only modifies set field values.
   */
  1: optional standard.ByteBuffer assign;

  /** Clear a given binary. */
  2: bool clear;

  /** Prepend to a given value. */
  8: standard.ByteBuffer prepend;

  /** Append to a given value. */
  9: standard.ByteBuffer append;
}

/**
 * The meaning of the patch op field ids, in all properly formulated patch
 * definitions.
 *
 * Patch field ids are interpreted at runtime, as a dynamic patch protocol,
 * without any additional schema derived from IDL patch definitions.
 */
@thrift.GenDefaultEnumValue
enum PatchOp {
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
   * A key/value-based remove for set/list, 'saturating subtract' for
   * numeric/'counting' types, and 'remove by key' for maps.
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
   * - Identical to 'add' for set;
   * - 'update or insert' for maps;
   * - 'append' for list, string or binary; and
   * - 'invert' for boolean.
   */
  Put = 9,
}

// The key of PatchOp::PatchPrior in ListPatch
@cpp.Adapter{
  name = "::apache::thrift::InlineAdapter<::apache::thrift::op::detail::ListPatchIndex>",
}
typedef i32 ListPatchIndex
