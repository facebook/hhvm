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
namespace android com.facebook.thrift.annotation_deprecated
namespace js thrift.annotation.thrift
namespace py.asyncio facebook_thrift_asyncio.annotation.thrift
namespace go thrift.annotation.thrift
namespace py thrift.annotation.thrift
namespace hs Facebook.Thrift.Annotation.Thrift

// start

/**
 * Indicates a definition/feature should only be used with permission, may
 * only work in specific contexts, and may change in incompatible ways without
 * notice. Note that this is primarily intended to annotate features by the Thrift Team
 * and isn't recommended for general use.
 */
@scope.Program
@scope.Definition
struct Experimental {}

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
struct TerseWrite {}

/** Indicates that an optional field's value should never be stored on the stack,
i.e. the subobject should be allocated separately (e.g. because it is large and infrequently set).

NOTE: The APIs and initialization behavior are same as normal field, but different from `@cpp.Ref`. e.g.

```
struct Foo {
  1: optional i32 normal;
  @thrift.Box
  2: optional i32 boxed;
  @cpp.Ref
  3: optional i32 referred;
}
```
in C++

```
Foo foo;
EXPECT_FALSE(foo.normal().has_value()); // okay
EXPECT_FALSE(foo.boxed().has_value()); // okay
EXPECT_FALSE(foo.referred().has_value()); // build failure: std::unique_ptr doesn't have has_value method

EXPECT_EQ(*foo.normal(), 0); // throw bad_field_access exception
EXPECT_EQ(*foo.boxed(), 0); // throw bad_field_access exception
EXPECT_EQ(*foo.referred(), 0); // okay, field has value by default
```

Affects C++ and Rust.
TODO: replace with @cpp.Box + @rust.Box
*/
@scope.Field
struct Box {}

/**
 * Indicates whether the nested fields are accessible directly.
 * https://github.com/facebook/fbthrift/blob/v2023.11.20.00/thrift/doc/idl/mixins.md
 */
@scope.Field
struct Mixin {}

/**
 * Option to serialize thrift struct in ascending field id order instead of field declaration order.
 *
 * This can potentially make serialized data size smaller in compact protocol,
 * since compact protocol can write deltas between subsequent field ids instead of full ids.
 *
 * NOTE: This annotation won't reduce payload size for other protocols.
 */
@scope.Struct
@Experimental // TODO(ytj): Release to Beta.
struct SerializeInFieldIdOrder {}

/**
 * Indicates an enum is a bitmask and should support bit-wise operators.
 * Currently generates additional code in C++ and Hack.
 */
@scope.Enum
struct BitmaskEnum {}

/**
 * Specifies the field where the exception message is stored.
 *
 * The "exception message" is typically a human-readable description of the
 * exception. It is made available to the exception-handling code via standard,
 * language-dependent APIs of the generated code, such as:
 *   - [`std::exception::what()`](https://en.cppreference.com/w/cpp/error/exception/what)
 *      in C++.
 *   - [`Throwable.getMessage()`](https://docs.oracle.com/javase/8/docs/api/java/lang/Throwable.html#getMessage--)
 *     in Java.
 *   - etc.
 *
 * This annotation can be specified on at most one field of an
 * [exception definition](https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#exceptions),
 * whose type must be `string`. The thrift compiler will generate an error
 * if this annotation is specified on a field in any other structured definition,
 * like a [struct definition](https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#structs)
 * or an [union definition](https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#unions)
 *
 * If an exception definition does not specify this annotation for any field, the
 * exception message returned by the aforementioned APIs is unspecified.
 */
@scope.Field
struct ExceptionMessage {}

/**
 * Indicates that a field's value should never be stored on the stack, and that
 * identical values can be shared in immutable contexts.
 */
@scope.Field
@Experimental
struct InternBox {}

/**
 * Indicates that an interaction's methods should be processed sequentially.
 */
@scope.Interaction
struct Serial {}

/**
 * Changes the URI of this definition away from the default-generated one.
 */
@scope.Enum
@scope.Service
@scope.Structured
struct Uri {
  1: string value;
}

/**
 * Changes the priority of this function (default NORMAL).
 */
@scope.Function
struct Priority {
  1: RpcPriority level;
}
enum RpcPriority {
  HIGH_IMPORTANT = 0,
  HIGH = 1,
  IMPORTANT = 2,
  NORMAL = 3,
  BEST_EFFORT = 4,
}

/**
 * States that the target structured type (struct, union or exception) is
 * "sealed", as defined in the
 * [Object Model](https://github.com/facebook/fbthrift/blob/main/thrift/doc/object-model/index.md#sealed-types).
 *
 * A sealed structured type can only have fields whose types are also sealed.
 *
 * In practice, this means that this type is safe to use as a map key type or as
 * a set element type, but any change to its schema (including changes that are
 * "typically" considered safe, such as adding a new field) MAY break backwards
 * compatiblity.
 *
 * Note that the tooling and environment in which Thrift IDL definitions live
 * (such as code repositories) may not be explicitly preventing such changes:
 * it is up to schema owners to be aware of the potential impact of schema
 * changes to sealed types.
 */
@scope.Structured
struct Sealed {}

/**
* Applies unstructured annotations to a definition.
*/
@scope.Definition
struct DeprecatedUnvalidatedAnnotations {
  1: map<string, string> items;
}

/**
* In addition to reserved words, Thrift reserves all identifiers
* that contain the case-insensitive substring fbthrift preceded
* by one or more underscores.
* The use of such identifiers requires users to explicitly annotate
* the usage with
*   `@thrift.AllowReservedFilename` for filenames
*   `@thrift.AllowReservedIdentifier` for all other identifiers
* and may result in undefined behavior.
*/
@scope.Definition
struct AllowReservedIdentifier {}

@scope.Program
struct AllowReservedFilename {}

/**
 * Applies to structured annotation that need to be accessed runtime in TypeSystem
 * and C++ always-on reflection.
 */
@scope.Struct
struct RuntimeAnnotation {}

/**
 * Allows the Thrift compiler to add a URI to the target typedef.
 *
 * Use of this annotation is strongly DISCOURAGED, and is provided for
 * backwards-compatibility purposes only.
 *
 * Indeed, Thrift IDL [typedefs](https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#typedefs)
 * do not correspond to the set of user-defined types that can have unique URIs
 * per the [Thrift Object Model](https://github.com/facebook/fbthrift/blob/main/thrift/doc/object-model/index.md#thrift-uri)
 * While it may seem like typedefs correspond to
 * [Opaque Alias Types](https://github.com/facebook/fbthrift/blob/main/thrift/doc/object-model/index.md#opaque-alias-types),
 * that is actually incorrect, as the "aliased" type that a typedef introduces
 * is considered identical - at the Object Model level - to the original type.
 *
 * This annotation is introduced to allow "grandfathering in" existing typedef
 * URIs in preparation for the thrift compiler to reject such cases in the
 * future (unless this annotation is specified).
 *
 * This annoation MUST NOT be applied to a typedef for which no URI is
 * specified (either explicitly via @thrift.Uri, or implicitly through a
 * non-empty
 * [package declaration](https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#package-declaration)).
 */
@scope.Typedef
struct AllowLegacyTypedefUri {}

/**
 * Allows the target field of a struct or exception, whose qualifier is
 * `optional`, to have a custom default value specified in IDL.
 *
 * Use of this annotation is strongly DISCOURAGED, as custom default values for
 * optional fields are both non-sensical and dangerous:
 *   - non-sensical because, by definition, the "default" state of an optional
 *     field is to have no value (i.e., be "absent") - as explicitly specified
 *     in the [Thrift Object Model](https://github.com/facebook/fbthrift/blob/main/thrift/doc/object-model/index.md#structured-types).
 *   - dangerous because in practice, the runtime behavior of the generated code
 *     for optional fields with custom default values is inconsistent (sometimes
 *     even for the same programming language!).
 *
 * This annotation is merely introduced to allow existing use cases to be
 * grandfathered into the new compiler validation logic, which will reject
 * optional fields with custom default values unless this annotation is
 * specified.
 *
 * This annotation MUST NOT be applied to a field whose qualifier is not
 * optional, or that doesn't have a custom default value. If applied, the target
 * field MUST be in a struct or exception (but NOT a union).
 */
@scope.Field
struct AllowUnsafeOptionalCustomDefaultValue {}

/**
 * Allows the target field of a union to have a custom default value specified
 * in IDL.
 *
 * Use of this annotation is strongly DISCOURAGED, for reasons similar to
 * `AllowUnsafeOptionalCustomDefaultValue` above, except that default values
 * for union fields make even less sense (imagine having multiple union fields
 * with custom default values!).
 *
 * This annotation is merely introduced to allow existing use cases to be
 * grandfathered into the new compiler validation logic, which will reject
 * union fields with custom default values unless this annotation is
 * specified.
 *
 * This annotation MUST NOT be applied to a field that doesn't have a custom
 * default value, or that is not in a union.
 */
@scope.Field
struct AllowUnsafeUnionFieldCustomDefaultValue {}

/**
 * Allows the target field of a struct or exception to be marked "required".
 *
 * Use of this annotation is strongly DISCOURAGED and almost certainly in
 * unexpected behavior. Indeed, the "required" field qualifier has been
 * deprecated since 2018, and the behavior in generated code is inconsistent
 * (most notably, it does NOT enforce any expectation on user code in C++ since
 * 2021).
 *
 * This annotation is merely introduced to allow existing use cases to be
 * grandfathered into the new compiler validation logic, which will otherwise
 * reject any "required" field.
 */
@scope.Field
struct AllowUnsafeRequiredFieldQualifier {}

/**
 * Allows Thrift IDL to build despite missing otherwise required Thrift URIs.
 *
 * According to the [Thrift Object Model](https://github.com/facebook/fbthrift/blob/main/thrift/doc/object-model/index.md#thrift-uri),
 * all user-defined types MUST have a unique, non-empty URI. In Thrift IDL, this
 * is accomplished by either:
 * 1. Specifying a non-empty [`package`](https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#package-declaration), or
 * 2. Explicitly adding the `@thrift.Uri` annotation with a non-empty `value`
 *    (see `struct Uri` above).
 *
 * Failure to do so results in an invalid Thrift schema, but this has
 * historically not been enforced - until H2'2026.
 *
 * Use of this annotation is strongly DISCOURAGED, but is provided to allow
 * existing schemas to be grandfathered in and continue building successfully
 * until the required URIs are specified.
 *
 * This annotation can be specified either on the declaration of types that are
 * required to have URIs (i.e., struct, union, exception and enum), or on the
 * file-level `package`.
 *
 * Adding this annotation on the `package` is equivalent to annotating every
 * type definiting in that file that is:
 * 1. required to have a URI (i.e., struct, union, exception or enum), and
 * 2. does not have a non-empty URI.
 *
 * A Thrift IDL schema is ill-formed if either of the following is true:
 * 1. It defines a type that:
 *     a. is required to have a URI, but the provided (or inferred) URI is empty, and
 *     b. is not annotated - directly or through its `package` - with this
 *        annotation (i.e., `@thrift.AllowLegacyMissingUris`)
 * 2. It uses this annotation unnecessarily, i.e. if:
 *    a. it specifies this annotation on a type definition that doesn't need it
 *       it (either because it has a non-empty URI, or is not required to have
 *       one), or
 *    b. it specifies this annotation on the `package` but does not have any
 *       type definition that needs it.
 */
@scope.Program
@scope.Structured
@scope.Enum
struct AllowLegacyMissingUris {}

/**
 * Allows the target `map<...>` (or `set<...>`) field, typedef or function
 * parameter to successfully build, despite the type of its keys (or elements)
 * not being sealed.
 *
 * Indeed, map key and set element types MUST be "sealed", as explained in the
 * [Object Model](https://github.com/facebook/fbthrift/blob/main/thrift/doc/object-model/index.md#sealed-types).
 *
 * A user-defined structured type can explicitly be marked as sealed by using
 * the `@thrift.Sealed` annotation.
 *
 * Use of this annotation is strongly DISCOURAGED, but is provided to allow
 * existing schemas (that were created prior to the concept of "sealed" types
 * being defined) to be grandfathered in and continue building successfully.
 */
@scope.Field
@scope.Typedef
@scope.FunctionParameter
struct AllowUnsafeNonSealedKeyType {}

/**
 * Marks a definition as deprecated.
 *
 * When applied, generated code will include language-specific deprecation
 * markers that produce compiler warnings when the deprecated element is used.
 *
 * Example:
 *   struct User {
 *     @thrift.Deprecated{message = "Use 'full_name' instead"}
 *     1: string name;
 *     2: string full_name;
 *   }
 */
@scope.Field
struct Deprecated {
  /**
   * Explanation of why this is deprecated and what to use instead.
   * This message will appear in compiler warnings.
   */
  1: string message;
}
