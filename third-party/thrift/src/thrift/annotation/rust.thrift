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

package "facebook.com/thrift/annotation/rust"

namespace java com.facebook.thrift.annotation.rust_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.rust
namespace go thrift.annotation.rust
namespace py thrift.annotation.rust

@scope.Field
@scope.Typedef
@scope.Struct
struct Adapter {
  // A fully qualified path to a struct that implements `fbthrift::adapter::ThriftTypeAdapter`.
  //
  // This will transform the type of this field to that struct's `AdaptedType`, and the corresponding
  // `ThriftTypeAdapter` methods will be called in the serialization and deserialization paths.
  //
  // Example:
  // If you have a Thrift struct like:
  // ```
  // struct Foo {
  //   @rust.Adapter{
  //    name = "fbthrift_adapters::DurationSecondsAdapter"
  //   }
  //   1: i64 duration_secs;
  // }
  // ```
  //
  // The generated Rust struct's `duration_secs` field will be set to the type
  // `<fbthrift_adapters::DurationSecondsAdapter as ::fbthrift::adapter::ThriftTypeAdapter>::AdaptedType`
  // (which is `std::time::Duration`) and marshalling the `i64` to and from `Duration` will be handled
  // in the serialization/deserialization path with the methods defined on the `ThriftTypeAdapter` impl.
  //
  // The name provided here must be a valid Rust path, i.e. the `fbthrift_adapters` crate must be added
  // to the Thrift library's `rust_deps`. Alternatively, you define the adapter in a file added to the
  // Thrift library's `rust_include_srcs`, and use the `crate::` prefix in your adapter name.
  //
  // If `<>` is present at the end of the name, we will treat the name as a generic and fill it in with
  // the original unadapted type.
  // For example:
  // ```
  // struct Foo {
  //   @rust.Adapter{
  //    name = "fbthrift_adapters::DurationSecondsAdapter<>"
  //   }
  //   1: i64 duration_secs;
  // }
  // ```
  // will use `fbthrift_adapters::DurationSecondsAdapter<i64>` as the adapter.
  1: string name;
} (thrift.uri = "facebook.com/thrift/annotation/rust/Adapter")
