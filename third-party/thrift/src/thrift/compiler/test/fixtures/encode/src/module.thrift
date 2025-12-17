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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.Experimental
package "facebook.com/thrift/test/fixtures/encode"

struct Foo {
  1: i32 field;
}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef Foo AdaptedFoo

@cpp.UseOpEncode
struct Bar {
  1: list<AdaptedFoo> list_field;
}

struct Baz {
  1: list<AdaptedFoo> list_field;
  2: list<list<AdaptedFoo>> nested_list_field;
}

enum Enum {
  first = 1,
  second = 2,
}

@cpp.UseOpEncode
struct OpEncodeStruct {
  1: i32 int_field;
  2: Enum enum_field;
  3: Foo foo_field;
  4: AdaptedFoo adapted_field;
  5: list<AdaptedFoo> list_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  6: optional list<AdaptedFoo> list_shared_ptr_field;
  7: list<AdaptedFoo> list_cpp_type_field;
  8: set<AdaptedFoo> set_field;
  9: map<AdaptedFoo, AdaptedFoo> map_field;
  10: map<i32, list<AdaptedFoo>> nested_field;
  11: Bar bar_field;

  @cpp.Adapter{name = "::FieldAdapter"}
  12: list<AdaptedFoo> adapted_list_field;
}
