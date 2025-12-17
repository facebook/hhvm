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

package "facebook.com/thrift/annotation/python"

namespace java com.facebook.thrift.annotation.python_deprecated
namespace android com.facebook.thrift.annotation.python_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.python
namespace go thrift.annotation.python
namespace py thrift.annotation.python

// start

/// Hides in thrift-py3 only, not in thrift-python
/// Allowed for all @scope.Definition, except for @scope.FunctionParameter as that would hide part of
/// the RPC function parameters.
@scope.RootDefinition
@scope.Field
@scope.Function
@scope.Transitive
struct Py3Hidden {}

/// Hides in thrift-py-deprecated only
@scope.Field
struct PyDeprecatedHidden {
  1: string reason;
}

@scope.Enum
struct Flags {}

@scope.Definition
struct Name {
  1: string name;
}

/// An annotation that applies a Python adapter to typedef or field, or directly on struct.
/// This completely replaces the underlying type of a thrift for a custom implementation and
/// uses the specified adapter to convert to and from the underlying Thrift type during (de)serialization.
///
/// Example 1:
///
///   @python.Adapter{name = "my.module.DatetimeAdapter", typeHint = "datetime.datetime"}
///   typedef i64 Datetime
///
/// Here the type 'Datetime' has the Python adapter `DatetimeAdapter`.
///
///
/// Example 2:
///
///   struct User {
///     @python.Adapter{name = "my.module.DatetimeAdapter", typeHint = "datetime.datetime"}
///     1: i64 created_at;
///   }
/// Here the field `created_at` has the Python adapter `DatetimeAdapter`.
///
///
/// Example 3:
///
///
///   @python.Adapter{name = "my.module.AnotherAdapter", typeHint = "my.module.AdaptedFoo"}
///   struct Foo {
///     1: string bar;
///   }
///
/// Here the struct `Foo` has the Python adapter `AnotherAdapter`.
///
@scope.Field
@scope.Typedef
@scope.Structured
struct Adapter {
  /// Fully qualified name of a Python adapter class, which should inherit from thrift.python.adapter.Adapter
  1: string name;
  /// Fully qualified type hint the above implementation adapts to.
  /// If ending with "[]", it becomes a generic, and the unadapted type will be filled between the brackets.
  2: string typeHint;
}

/// Controls cpp <-> python FFI for a struct or union
/// By default, struct uses marshal C API unless cpp.Type or cpp.Adapter is present
/// on a field or a type
/// Use this annotation to opt-in struct to marshal in spite of cpp.Type or cpp.Adapter
/// Alternatively, use this struct with serialize = false to use serialization for FFI.
@scope.Structured
struct UseCAPI {
  1: bool serialize = false;
}

/// Enables C++ Adapter for thrift-py3. It treats C++ Adapter on typedef as cpp.Type.
/// It is only available for typedefs that resolve to binary, string, and container type.
@scope.Typedef
struct Py3EnableCppAdapter {}

/// Allows inheritance from a struct or exception in thrift-py3.
/// Inheritance from union is DEPRECATED!
/// Do not add new usage of this. Prefer composition over inheritance.
@scope.Struct
@scope.Exception
struct MigrationBlockingAllowInheritance {}

/// Enables sorted order for a field with `set` type.
/// Only affects serialization for thrift-python and thrift-py3.
/// Note that `set` in thrift-python has no stable ordering once deserialized.
/// DO NOT RELY on this. Brittle tests that rely on this will eventually be disabled.
@scope.Field
struct DeprecatedSortSetOnSerialize {}

/// Enables key-sorted order for a field with `map` type.
/// Only affects thrift-python and thrift-py3.
/// Note that key sorting only occurs on serialization, and not on deserialization.
/// DO NOT RELY on this. Brittle tests that rely on this will eventually be disabled.
@scope.Field
struct DeprecatedKeySortMapOnSerialize {}

/// Disable caching all fields for a struct.
/// Also available as a thrift_library compiler option:
///     thrift_library(..., thrift_python_options = ["disable_field_cache"])
///
/// Has NO effect in cinder runtime (e.g., IG Django)
/// Only affects thrift-python, not older deprecated variants.
///
/// Usage guidelines:
///   - Improves latency/throughput when struct fields accessed only once.
///   - Reduces memory usage for most use cases; try this to resolve OOMs.
///   - Worsens latency for subsequent field accesses relative to default.
@scope.Struct
struct DisableFieldCache {}

/// An enum that specifies the constraint behavior on a field.
///
/// In this context, "constraint" refers to any logic applied by
/// Thrift-provided types and runtime libraries on native (Python)
/// operations that do not fully comply with the Thrift Object Model,
/// but could otherwise be valid Python.
///
/// An example of such an operation (and the first concrete use of
/// this annotation) is the handling of 32-bit floating point numbers:
/// native Python `float`s all have the same width, which is
/// typically 64 bits, whereas Thrift's `float` type is 32 bit
/// (compared to `double`, which is 64 bits). Writing a 64 bit
/// Python float to a 32-bit Thrift float is an example of a
/// potentially non-compliant operation, that may require a constraint
/// to be applied - implicitly or explicitly.
enum ConstraintLevel {
  /// UNSPECIFIED: Intrinsic default value for this enum. Should never be assigned
  /// explicitly. The actual constraint level applied on `UNSPECIFIED`
  /// depends on the operation, and is documented below.
  UNSPECIFIED = 0,

  /// ALLOW_INVALID: applies the lowest level of validation on struct creation or
  /// mutation. While some basic type checking may be performed,
  /// does not fully validate that the value conforms to the
  /// [Thrift Object Model](https://github.com/facebook/fbthrift/blob/main/thrift/doc/object-model/index.md#primitive-types).
  ///
  /// Native (Python) operations with this constraint level SHOULD NOT
  /// immediately fail, but MAY result in non-compliant Thrift values.
  /// Subsequent operations on these values (eg. serialization) MAY fail.
  ALLOW_INVALID = 1,

  /// MAP: maps the input value to the closest conforming value. For example,
  /// thrift-python MAPs integer `float` values to `i64` fields.
  ///
  /// Native (Python) operations with this constraint level SHOULD NOT
  /// immediately fail, and MUST produce a result that is compliant with the
  /// Thrift Object Model. That result MAY not be the same as the originally
  /// intended change by the user.
  MAP = 2,

  /// REJECT: applies the strictest level of validation on struct creation or
  /// mutation. Raises an error for input types that do not exactly
  /// conform to the Thrift Object Model. For example, integers
  /// greater than INT64_MAX cause OverflowError when used with a Thrift
  /// `i64` field.
  ///
  /// Native (Python) operations with this constraint level MAY fail. If they do
  /// not fail, they MUST produce a result that is both (1) compliant with the
  /// Thrift Object Model, and (2) identical to the user's intent.
  REJECT = 3,
}

/// Allows custom constraint on Thrift `float` fields, i.e., 32-bit floating-
/// point values, during Python thrift struct initialization or mutation.
///
/// This is necessary because Python's native floating-point number type
/// (`float`) may have more precision that 32 bits. Indeed, while the exact
/// precision is implementation-specified, it typically corresponds to `double`
/// in C, i.e. 64 bits (see
/// https://docs.python.org/3/library/stdtypes.html#typesnumeric). Ensuring that
/// such native values are valid 32-bit Thrift `float`s requires them to be
/// properly constrained, by either:
/// 1. rounding them to the closest 32-bit number, if they are in range, or
/// 2. bounding them to +/-Inf if they are greater/less than the max/min
///    representable 32-bit number.
///
/// By default, in the absence of this annotation, thrift-python types ensure
/// that all values assigned to (or accessed from) single precision
/// floating-point (i.e., `float` in Thrift IDL) fields have the correct
/// precision, constraining them as needed.
///
/// Note that NaN is *never* a valid Thrift floating point number, as specified
/// in the [Thrift Object Model](https://github.com/facebook/fbthrift/blob/main/thrift/doc/object-model/index.md#primitive-types).
/// The behavior of Thrift operations in presence of native Python NaN values is
/// left undefined.
///
/// The behavior may be customized using each field of this struct. The defaults
/// correspond to the default behavior if this annotation is not applied to the
/// `float` type.
///
/// This annotation MUST NOT be applied on fields whose [Thrift IDL Type](https://github.com/facebook/fbthrift/blob/main/thrift/doc/glossary/kinds-of-types.md#thrift-idl-types)
/// is not `float`, or a container whose item type(s) are not `float` (or
/// containers that satisfy this property, recursively).
@scope.Typedef
struct ConstrainedFloat32 {
  ///   - `precision_loss`: controls handling of `float` in the range
  ///     (-MAX_FLOAT_32, +MAX_FLOAT_32).
  ///     * ALLOW_INVALID: no rounding is applied at struct creation, but MAP
  ///       applies on serialization.
  ///     * MAP: rounded to the nearest valid 32-bit float according to IEEE754
  ///       trunctation convention.
  ///     * REJECT: raises `TypeError` if the type is not already a 32-bit float
  1: ConstraintLevel precision_loss;
  ///   - `inf_overflow`: controls handling of `float` outside the range
  ///     [-MAX_FLOAT_32, +MAX_FLOAT_32].
  ///     NOTE: values that are already +/- `Inf` are unaffected by this option.
  ///     * ALLOW_INVALID: the value is preserved on struct creation, but MAP
  ///       applies on serialization.
  ///     * MAP: rounded "up" to +/- Inf according to sign, according to IEEE754
  ///       convention.
  ///     * REJECT: raises `OverflowError` if the type is outside the range
  ///       [-MAX_FLOAT_32, +MAX_FLOAT_32], unless the value is already
  ///        +/- `Inf`, in which case it's preserved.
  2: ConstraintLevel inf_overflow;
  ///   - `not_a_number`: controls handling of special `NaN` values.
  ///     * ALLOW_INVALID: the value is preserved on struct creation and serialization.
  ///     * MAP: same as ALLOW_INVALID.
  ///     * REJECT: raises `TypeError`for `NaN` values.
  3: ConstraintLevel not_a_number;
}
