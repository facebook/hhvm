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
  //
  // If the adapter name starts with `crate::` and this `@rust.Adapter` is applied transitively with
  // @scope.Transitive, `crate::` will be replaced with the name of the crate in which the
  // transitive annotation is defined.
  1: string name;
} (thrift.uri = "facebook.com/thrift/annotation/rust/Adapter")

@scope.Enum
@scope.Struct
struct Derive {
  // List of additional derives to apply to the generated struct.
  //
  // Example:
  // ```
  // @rust.Derive{ derives = ["Foo"] }
  // struct SomeStruct {
  //    1: string some_field;
  // }
  // ```
  // will generated the Rust struct
  // ```
  // #[derive(Foo)]
  // struct SomeStruct {
  //    some_field: String,
  // }
  // ```
  //
  // If the derive starts with `crate::` and this `@rust.Derive` is applied transitively with
  // @scope.Transitive, `crate::` will be replaced with the name of the crate in which the
  // transitive annotation is defined.
  1: list<string> derives;
} (thrift.uri = "facebook.com/thrift/annotation/rust/Derive")

@scope.Function
@scope.Service
struct ServiceExn {
  // If `true`, this allows returning an `anyhow::Error` from within a Thrift server handler method,
  // and that `anyhow::Error` will be turned into an `ApplicationException`. This is similar in behavior
  // to what happens if you throw an unhandled exception type in Python Thrift server or C++ Thrift server.
  //
  // The `ApplicationException` returned will have the error code `Unknown` and message
  // `format!("{:#}", anyhow_err)`.
  //
  // NOTE: it is generally considered bad practice to use this because it eliminates the ability
  // to match on specific error types on the client side. When possible, it is recommended you
  // always return structured error types (though it is more verbose). This annotation is provided
  // solely for convenience and should not be used services where error type matching is needed.
  //
  // Example, the following Thrift:
  // ```
  // service Foo {
  //   @rust.ServiceExn{ anyhow_to_application_exn = true }
  //   void bar();
  // }
  // ```
  // would allow for the following Rust server impl:
  // ```
  // #[async_trait]
  // impl Foo for FooServerImpl {
  //     async fn bar() -> Result<(), BarExn> {
  //         if some_condition {
  //             Err(anyhow::anyhow!("some error"))?
  //         }
  //
  //         // Note you must always convert to `anyhow::Error` first for non-`anyhow::Error`
  //         // types.
  //         some_client_call.await.context("failed some_client_call")?;
  //
  //         // you can still return a structured exn type if desired
  //         return Err(BarExn::ie(...));
  //     }
  // }
  // ```
  //
  // You can also use this annotation on the service definition itself to have it apply to all
  // methods on the service, e.g.
  // ```
  // @rust.ServiceExn{ anyhow_to_application_exn = true }
  // service Foo {
  //   // Both `bar` and `baz` will support `anyhow::Error` -> `ApplicationException`.
  //   void bar();
  //   void baz();
  // }
  // ```
  1: bool anyhow_to_application_exn;
} (thrift.uri = "facebook.com/thrift/annotation/rust/ServiceExn")
