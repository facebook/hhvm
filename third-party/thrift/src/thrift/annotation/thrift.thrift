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

include "thrift/annotation/scope.thrift"

package "facebook.com/thrift/annotation"

namespace java com.facebook.thrift.annotation_deprecated
namespace js thrift.annotation.thrift
namespace py.asyncio facebook_thrift_asyncio.annotation.thrift
namespace go thrift.annotation.thrift
namespace py thrift.annotation.thrift

/** Indicates a definition/feature may change in incompatible ways. */
@scope.Program
@scope.Definition
struct Beta {}

/**
 * Indicates a definition/feature should only be used with permission, may
 * only work in specific contexts, and may change in incompatible ways without
 * notice.
 */
@scope.Program
@scope.Definition
struct Experimental {}

/**
 * Indicates a definition/feature should only be used in an ephemeral testing
 * enviornment.
 *
 * Such enviornments only store serialized values temporarly and strictly
 * control which versions of Thrift definitions are used, so 'compatibility'
 * is not a concern.
 */
@scope.Program
@scope.Definition
struct Testing {}

/**
 * Annotate a thrift structured or enum to indicate if ids or values should not
 * be used.
 *
 * For example, you may want to mark ids as deprecated, or these ids
 * might be reserved for other use cases or annotations.
 *
 * The resolved set of disallowed ids is the union of the values in `ids` and
 * the range of values represented in `id_ranges`. Example:
 *
 *  // These ids are not allowed: 3, 8, half-open ranges [10, 15), [20, 30)
 *  @thrift.ReserveIds{ids = [3, 8], id_ranges = {10: 15, 20: 30}}
 *  struct Foo {
 *    ...
 *    3: i64 f; // Build failure: 3 cannot be used
 *  }
 */
@scope.Structured
@scope.Enum
struct ReserveIds {
  /** Individual ids that cannot be used. */
  1: list<i32> ids;

  /**
   * Represents ranges of ids that cannot be used.
   *
   * Each (key: value) pair represents the half-open range `[key, value)`,
   * where `key` is included and `value` is not. For example, the map
   * `{10: 15, 20: 30}` represents the union of id/value ranges `[10, 15)` and
   * `[20, 30)`.
   */
  2: map<i32, i32> id_ranges;
}

/**
 * Indicates additional backward compatibility restrictions, beyond the
 * standard Thrift required 'wire' compatibility.
 */
// TODO(afuller): Hook up to backward compatibility linter.
@scope.Structured
@Experimental // TODO: Fix naming style.
struct RequiresBackwardCompatibility {
  1: bool field_name = false;
}

/** Disables testing features. */
@scope.Program
@scope.Definition
struct NoTesting {}

/** Disables experimental features. */
@scope.Program
@scope.Definition
struct NoExperimental {}

/** Disables @Beta features. */
@NoExperimental // Implies no experimental
@scope.Transitive
struct NoBeta {}

/**
 * Indicates a definition/feature must not depend directly on an unreleased
 * or testing definition/feature.
 */
@NoBeta
@NoTesting
@scope.Transitive
struct Released {}

////
// Thrift feature annotations.
////

/**
 * An annotation that changes the field qualifier from 'none' to 'terse'.
 * A terse field is eligible to skip serialization, when it equals to the
 * intrinsic default value. It also clears to the intrinsic default value
 * before deserialization to distinguish between if a terse field was skipped
 * or missing during serialization. This is different from an unqualified
 * field, as an unqualified field is always serialized regardless of its value,
 * and it is not cleared before deserialization.
 *
 * The annotation can be only used to annotate an unqualified field, and when
 * it is annotating a struct or exception, it changes all unqualified fields to
 * terse fields. Note, the annotation can not be used for union.
 */
@scope.Program
@scope.Struct
@scope.Exception
@scope.Field
@Experimental
struct TerseWrite {}

/** Indicates that a field's value should never be stored on the stack. */
@scope.Field
@Beta
struct Box {}

// TODO(ytj): Document.
@scope.Field
@Beta
struct Mixin {}

/**
 * Option to serialize thrift struct in ascending field id order.
 *
 * This can potentially make serialized data size smaller in compact protocol,
 * since compact protocol can write deltas between subsequent field ids.
 */
@scope.Struct
@Experimental // TODO(ytj): Release to Beta.
struct SerializeInFieldIdOrder {}

/**
 * Indicates an enum is a bitmask and should support bit-wise operators.
 */
@scope.Enum
@Experimental // TODO: Support in C++, Python, Java.
struct BitmaskEnum {}

////
// Thrift version annotations.
////

/** Enables all released v1 features. */
// TODO: Release features ;-).
@scope.Transitive
struct v1 {}

// TODO(dokwon): Re-enable v1 features.
/**
 * Enables all beta v1 features.
 *
 * Beta features are guaranteed to *not* break unrelated Thrift features
 * so they should be relatively safe to test alongside other beta or
 * released Thrift features.
 */
@v1 // All v1 features.
@Beta // All uses of v1beta inherit `@Beta`.
// @TerseWrite
@scope.Transitive
struct v1beta {}

/**
 * Enables all experimental v1 features.
 *
 * Use with *caution* and only with explicit permission. This may enable
 * features may change significantly without notice or not work correctly
 * in all contexts.
 */
@v1beta // All v1beta features.
@SerializeInFieldIdOrder
@Experimental // All uses of v1alpha inherit `@Experimental`.
@scope.Transitive
struct v1alpha {}

/**
 * Enables experimental features, even those that are known to break common
 * use cases.
 */
@v1alpha // All v1alpha features.
@Testing // Should only be used in tests.
@scope.Transitive
struct v1test {}

/**
 * Specifies the field where the exception message is stored. The field
 * is used to generate an additional method to get it.
 */
// TODO(afuller): Consider allowing this annotation to be placed on the field
// itself, so the annotation can be used (transitively), without specifying an
// explicit name. Also, maybe move to api.thrift.
@scope.Exception
@Experimental // TODO: Support in C++, Python, Java.
struct ExceptionMessage {
  1: string field;
}

/**
 * Generates a const of type schema. Struct containing the schema of the
 * annotated type. Optionally specify name to override default
 * schema<structName>.
 */
@scope.Structured
@scope.Service
@scope.Const
@scope.Enum
@scope.Typedef
@Experimental
struct GenerateRuntimeSchema {
  1: string name;
}

/**
 * Indicates that a field's value should never be stored on the stack, and that
 * identical values can be shared in immutable contexts.
 */
@scope.Field
@Experimental
struct InternBox {}
