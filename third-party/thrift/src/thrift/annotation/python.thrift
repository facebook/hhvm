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
namespace py.asyncio facebook_thrift_asyncio.annotation.python
namespace go thrift.annotation.python
namespace py thrift.annotation.python

// start

/// Hides in thrift-py3 only, not in thrift-python
@scope.Definition
struct Py3Hidden {}

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
} (thrift.uri = "facebook.com/thrift/annotation/python/Adapter")

/// Controls cpp <-> python FFI for a struct or union
/// By default, struct uses marshal C API unless cpp.Type or cpp.Adapter is present
/// on a field or a type
/// Use this annotation to opt-in struct to marshal in spite of cpp.Type or cpp.Adapter
/// Alternatively, use this struct with serialize = false to use serialization for FFI.
@scope.Structured
struct UseCAPI {
  1: bool serialize = false;
} (thrift.uri = "facebook.com/thrift/annotation/python/UseCAPI")
