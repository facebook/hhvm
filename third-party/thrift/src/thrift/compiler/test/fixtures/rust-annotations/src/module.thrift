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

include "thrift/annotation/rust.thrift"

// --

@rust.Type{name = "OrderedFloat<f64>"}
typedef double double_t
struct T0 {
  1: double_t data;
}
struct T1 {
  @rust.Type{name = "OrderedFloat<f64>"}
  1: double data;
}
@rust.Type{name = "sorted_vector_map::SortedVectorMap"}
typedef map<string, i64> map_t
struct T2 {
  1: map_t data;
}
struct T3 {
  @rust.Type{name = "sorted_vector_map::SortedVectorMap"}
  1: map<string, i64> data;
}
struct T4 {
  @rust.Type{name = "HashMap"}
  1: map_t data;
}

// --

@rust.Type{name = "smallvec::SmallVec<[u8; 16]>"}
typedef binary binary_t (rust.newtype)

@rust.NewType
typedef i64 Generation

@rust.NewType
@rust.Type{name = "smallvec::SmallVec<[u8; 16]>"}
typedef binary binary_t2

// --

@rust.Ord // This struct would not derive `Ord` but for this.
struct T5 {
  1: map<i32, string> data;
}

// --

@rust.Exhaustive // no `pub _dot_dot_Default_default: self::dot_dot::OtherFields`
struct T6 {}

// --

struct T7 {
  @rust.Box
  1: i64 data; // `::std::boxed::Box<::std::primitive::i64>`
}

// --

struct T8 {
  @rust.Arc
  1: i64 data; // `::std::sync::Arc<::std::primitive::i64>`
}

// --

@rust.RequestContext
service S1 {
  T6 r();
}

// --

@rust.Copy
struct T9 {
  1: i64 data;
}

// --

@rust.Derive{derives = ["Foo", "crate::Bar"]}
@scope.Transitive
struct TransitiveDerives {}

@rust.Derive{derives = ["Foo"]}
enum EnumWithDerives {
  UNKNOWN = 0,
  STUFF = 420,
}

@TransitiveDerives
struct StructWithTransitiveDerives {}

safe transient server exception SomeError {}

@rust.ServiceExn{anyhow_to_application_exn = true}
service AllMethods {
  void foo();
  string bar() throws (1: SomeError se);
}

service OneMethod {
  @rust.ServiceExn{anyhow_to_application_exn = true}
  void foo();
  string bar() throws (1: SomeError se);
}

@rust.ServiceExn{anyhow_to_application_exn = true}
service OneMethodOptOut {
  void foo();
  @rust.ServiceExn{anyhow_to_application_exn = false}
  string bar() throws (1: SomeError se);
}

union Bar {
  1: i32 WithAnnotation (rust.name = "Annotated");
  2: i32 WithoutAnnotation;
}

// @lint-ignore THRIFTCHECKS
const Bar BAR1 = {WithAnnotation: 123};
// @lint-ignore THRIFTCHECKS
const Bar BAR2 = {WithoutAnnotation: 123};
