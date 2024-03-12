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
include "thrift/lib/thrift/id.thrift"

cpp_include "thrift/lib/cpp2/op/detail/Patch.h"

@thrift.TerseWrite
@thrift.Experimental
package "facebook.com/thrift/op"

namespace cpp2 apache.thrift.op
namespace py3 apache.thrift.op
namespace java com.facebook.thrift.op
namespace java.swift com.facebook.thrift.op
namespace js apache.thrift.op
namespace py.asyncio apache_thrift_asyncio.patch
namespace go thrift.lib.thrift.patch
namespace py thrift.lib.thrift.patch

typedef id.FieldId FieldId

/**
 * An annotation that indicates a patch representation should be generated for
 * the associated definition.
 *
 * This is deprecated and we should use the new codegen workflow instead.
 */
@scope.Program
@scope.Structured
struct GeneratePatch {}

/**
 * An annotation that indicates a patch representation should be generated for
 * the associated definition. Similar to `GeneratePatch` but only works for new
 * codegen workflow and it won't work on package level.
 */
@scope.Structured
struct GeneratePatchNew {}

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

// TODO change the element type to FieldId
@cpp.Adapter{name = "::apache::thrift::op::detail::FieldIdListToSetAdapter"}
typedef list<i16> FieldIdList
