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
namespace py.asyncio facebook_thrift_asyncio.annotation.thrift
namespace go thrift.annotation.thrift
namespace py thrift.annotation.thrift

////
// Thrift release state annotations.
////

/** Indicates a definition/feature may change in incompatible ways. */
@scope.Program
@scope.Definition
struct Beta {}

/**
 * Indicates a definition/feature should only be used with permission, may only
 * work in specific contexts, and may change in incompatible ways without notice.
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

/** Indicates a definition/feature should no longer be used. */
// TODO(afuller): Add a validator to produce warnings when annotated definitions
// are used.
@Beta // TODO(afuller): Hook up to code gen.
@scope.Program
@scope.Definition
struct Deprecated {
  1: string message;
}

// Annotate a thrift structured or enum to indicate if ids should not be
// used. For example, you may want to mark ids as deprecated, or these ids
// might be reserved for other use cases or annotations.
//
// The resolved set of disallowed ids is the union of the values in `ids` and
// the range of values represented in `id_ranges`. Example:
//
//  // These ids are not allowed: 3, 8, half-open ranges [10, 15), [20, 30)
//  @thrift.ReserveIds{ids = [3, 8], id_ranges = {10: 15, 20: 30}}
//  struct Foo {
//    ...
//    3: i64 f;       // Build failure: 3 cannot be used
//  }
@scope.Structured
@scope.FbthriftInternalEnum
struct ReserveIds {
  // Individual ids that cannot be used
  1: list<i32> ids;
  // Represents ranges of ids that cannot be used. Each (key: value) pair
  // represents the half-open range [key, value) -- key is included and
  // value is not.
  // Example: {10: 15, 20: 30} represents the range [10, 15) and [20, 30)
  2: map<i32, i32> id_ranges;
}

/**
 * Indicates  a definition/feature will be removed in the next release.
 *
 * Pleased migrate off of all @Legacy as soon as possible.
 */
// TODO(afuller): Add a linter to produce errors when annotated definitions
// are used.
@Deprecated // Legacy implies deprecated.
@scope.Transitive
struct Legacy {
  1: string message;
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

/** Best-effort disables experimental features. */
@scope.Program
@scope.Definition
struct NoExperimental {}

/** Best-effort disables @Beta features. */
@NoExperimental // Implies no experimental
@scope.Transitive
struct NoBeta {}

/**
 * Best-effort disables @Legacy features.
 */
// TODO(ytj): Everyone should be able to test without legacy features. Fix
// compatibility with legacy reflection and move to @Beta.
@scope.Program
@scope.Definition
@Experimental
struct NoLegacy {}

/**
 * Best-effort disables @Deprecated features.
 *
 * Should only be enabled in `test` versions, as deprecated implies removing
 * the feature will break current usage (otherwise it would be @Legacy or
 * deleted)
 */
@NoLegacy // Implies NoLegacy
@Beta // Everyone should be able to test without deprecated features.
@scope.Transitive
struct NoDeprecated {}

////
// Thrift feature annotations.
////

// TODO(dokwon): Fix code gen and release to Beta.
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

// TODO(dokwon): Document.
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
 * Adds a default enum value (0), with the given name, if one is not
 * already defined.
 *
 * All v1+ enums must have an explicitly defined default value (0).
 * This annotation automatically adds such a value if not already present.
 */
// TODO(afuller): Add validation which produces an error when a @NoLegacy enum
// doesn't have a default value defined.
// TODO(afuller): Consider updating code generators to use the same name
// they use for empty/nil/null in unions, when a zero value is not specified.
@scope.FbthriftInternalEnum
@scope.Program
@Beta
struct GenDefaultEnumValue {
  /**
   * The name to use for the generated enum value.
   *
   * This intentionally does **not** use the most common 'zero' enum value name,
   * 'Default', by default; as, defining a `Default = 0` enum value explicitly
   * is a useful means of self-documenting that setting an explicit value is
   * never required. In which case, it is part of the API, and should not be
   * removed in favor of an implicitly generated value.
   *
   * On the other hand, 'Unspecified' clearly indicates that the requirements
   * are not intrinsic to the enum. In which case, the relevant documentation
   * should be consulted (e.g. the doc strings on the function or field).
   */
  1: string name = "Unspecified";
}

////
// Thrift version annotations.
////

/** Enables all released v1 features. */
// TODO: Release features ;-).
@scope.Transitive
struct v1 {}

/**
 * Enables all beta v1 features.
 *
 * Beta features are guaranteed to *not* break unrelated Thrift features
 * so they should be relatively safe to test alongside other beta or
 * released Thrift features.
 */
@v1 // All v1 features.
@GenDefaultEnumValue
@NoLegacy // Disables features that will be removed.
@Beta // All uses of v1beta inherit `@Beta`.
@TerseWrite
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
@NoDeprecated // Remove deprecated features by default for tests.
@v1alpha // All v1alpha features.
@Testing // Should only be used in tests.
@scope.Transitive
struct v1test {}

/**
 * Specifies the field where the exception message is stored. The field
 * is used to generate an additional method to get it.
 */
// TODO: Support in C++, Python, Java.
@scope.Exception
struct ExceptionMessage {
  1: string field;
}

/**
 * Specifies if the enum is a bitmask.
 */
// TODO: Support in C++, Python, Java.
@scope.Enum
struct BitmaskEnum {}
/**
 * Generates a const of type schema.Struct containing the schma of the
 * annotated struct. Optionally specify name to override default
 * schema<structName>.
 */
@scope.Struct
@Experimental
struct GenerateRuntimeSchema {
  1: string name;
}
