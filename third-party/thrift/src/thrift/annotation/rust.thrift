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
namespace android com.facebook.thrift.annotation.rust_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.rust
namespace go thrift.annotation.rust
namespace py thrift.annotation.rust

@scope.Typedef
struct NewType {
// # `rust.NewType`
//
// Make a newtype from a typedef. For example,
// ```
// @rust.NewType
// typedef binary Sha1
// ```
// will result in `pub struct Sha1(pub std::vec::Vec<u8>)`.
//
// Another common idiom is to use `rust.Type` and `rust.NewType` together like
// this:
// ```
// @rust.NewType
// @rust.Type { name =  "smallvec::SmallVec<[u8; 20]>" }
// typedef binary Sha1
// ```
// in this case we'll get `pub struct Sha1(smallvec::SmallVec[u8; 20])`.
}

@scope.Field
@scope.Typedef
struct Type {
  // # `rust.Type`
  //
  // There is a default Rust type associated with each Thrift type. For
  // example, the default Rust type to represent Thrift `map<>`s is
  // `std::collections::BTreeMap<>`.
  //
  // The `rust.Type` annotation provides an ability to "override"
  // (substitute) a non-default Rust type in certain circumstances (full
  // details below). We might say for example, `@rust.Type{name="HashMap"}`
  // to override an instance of a specific Thrift `map<>`.
  //
  // The `rust.Type` annotation can be applied to any type but has no
  // effect when applied to `string`, `list<>`, `struct` or `enum` types.
  //
  // The `name` argument of a `rust.Type` annotation may specify a
  // "standard" or "nonstandard" type: a name containing a '`::`' is
  // classified as a nonstandard type whereas, a name without a '`::`' is
  // classified as standard.
  //
  // Standard types that may appear in `@rust.Type` annotations are exactly
  // types that are (re-)exported from the `fbthrift::builtin_types`
  // module. For such types, the `fbthift` package provides stock
  // `fbthrift::Serialize<>` and `fbthrift::Deserialize<>` instances for
  // them. At the current time the full set of such types is
  // `std::collections::*`, `bytes::Bytes` and
  // `ordered_float::OrderedFloat`.
  //
  // This is an example of an application of a `@rust.Type` annotation with a
  // standard type:
  // ```
  //   @rust.Type { name = "OrderdedFloat<f64>" }
  //   typedef double Double
  //   struct T { 1: Double data; } // `data : fbthrift::builtin_types::OrderedFloat<f64>`
  // ```
  //
  // This is an example of application of a `@rust.Type` annotation with a
  // nonstandard type:
  // ```
  //   @rust.Type { name = "smallvec::SmallVec<[u8; 32]>" }
  //   typedef binary binary_t
  //   struct T { 1: binary_t data; } // `data : smallvec::SmallVec<[u8; 32]>`
  // ```
  //
  // Nonstandard types, when they appear in `@rust.Type` annotations
  // applied to Thrift `map<>`, `set<>` or `binary` types will result in
  // the generation of `fbthrift::Serialize<>` and
  // `fbthrift::Deserialize<>` instances for those types. The serialization
  // code makes assumptions about valid expressions and the existence of
  // trait implementations for such types that are documented below.
  //
  // A nonstandard type say can also be applied to `i64`. In this case, the
  // resulting generated serialization code assumes the existence of
  // `fbthrift::Serialize<>` and `fbthrift::Deserialize<>` for that
  // nonstandard type.
  //
  // A nonstandard type applied to Thrift `void`, `bool`, `float`, `byte`,
  // `i16`, `i32`, `double`, and `float` types will not result in the
  // generation of any serialization code for the nonstandard type
  // (rendering nonstandard types applied to these types effectively
  // unsupported at this time).
  //
  // "Codegen" errors or bugs resulting from the use of standard types in
  // valid positions in `@rust.Type` annotations should be considered the
  // responsibility of the the Rust Thrift maintainers to address. Less
  // "formal" support should be expected from the Rust Thrift maintainers
  // when nonstandard types are involved.
  //
  // ## `binary`
  //
  // The default Rust type for a Thrift `binary` is
  // `std::vec::Vec<std::primitive::u8>`. An example override:
  // ```
  //   @rust.Type{name = "smallvec::SmallVec<[u8; 32]>"}
  //   typedef binary binary_t
  //   struct T {
  //     1: binary_t data;
  //   }
  // ```
  //
  // If nonstandard `B` models Thrift `binary`, `b : B`, `other: &[u8]` and
  // `vec : std::vec::Vec<u8>` then the following expressions are required
  // to be valid and the following trait instances must exist:
  //
  // | expression                                            |
  // | :---------------------------------------------------- |
  // | `let _: &[u8]  = b.as_slice()`                        |
  // | `let _: B = <B>::with_capacity(l)`                    |
  // | `b.extend_from_slice(other)`                          |
  //
  // | type           | traits                               |
  // | :------------- | :----------------------------------  |
  // | `B`            | `Debug`, `Default`,                  |
  // |                | `From<std::vec::Vec<u8>>`            |
  //
  // ## `set`
  //
  // The default Rust type for a thrift `set` is
  // `std::collections::BTreeSet<>`. An example override:
  // ```
  //   @rust.Type{name = "sorted_vector_map::SortedVectorSet"}
  //   typedef set<string> set_t
  //   struct T {
  //     1: set_t data; // data : sorted_vector_map::SortedVectorSet<string>
  //   }
  // ```
  //
  // If nonstandard `S` models thrift `set`, `K` is the Rust element type,
  // `k : K`, `l : usize`, `s : S<K>` and `'a` a lifetime, required valid
  // expressions and trait implementations are as follows:
  //
  // | expression                                            |
  // | :---------------------------------------------------- |
  // | `for _ in &s { ... }`                                 |
  // | `let _: usize = s.len()`                              |
  // | `let mut _: T<K> = <S<K>>::with_capacity(l);`         |
  // | `s.insert(k)`                                         |
  //
  // | type           | traits                               |
  // | :------------- | :----------------------------------  |
  // | `S<K>`         | `Debug`, `Default`                   |
  // | `&'a S<K>`     | `IntoIterator<Item = &'a K>`         |
  //
  // ## `map`
  //
  // The default rust type for a thrift `map` is
  // `std::collections::BTreeMap<>`. An example override:
  // ```
  //  @rust.Type{name = "sorted_vector_map::SortedVectorMap"}
  //  typedef map<string, i64> map_t
  //  struct T {
  //    1: map_t data; // data: sorted_vector_map::SortedVectorMap<string, i64>
  //  }
  // ```
  //
  // If nonstandard `T` models thrift `map`, `K` and `V` are the Rust map
  // key and value types respectively, `k : K`, `v : V`, `l : usize`, `m :
  // T<K, V>` and `'a` a lifetime, required valid expressions and trait
  // implementations are:
  //
  // | expression                                            |
  // | :---------------------------------------------------- |
  // | `for (key, val) in &m { ... }`                        |
  // | `let _: usize = m.len()`                              |
  // | `let mut _: T<K, V> = <T<K, V>>::with_capacity(l);`   |
  // | `m.insert(k, v)`                                      |
  //
  // | type           | traits
  // | :------------- | :----------------------------------
  // | `T<K, V>`      | `Debug`, `Default`
  // | `&'a T<K, V>`  | `IntoIterator<Item = (&'a K, &'a V)>`

  1: string name;
}

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
