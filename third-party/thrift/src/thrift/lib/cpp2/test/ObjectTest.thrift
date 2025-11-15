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

cpp_include "thrift/test/AdapterTest.h"

package "facebook.com/thrift/lib/test"

struct Foo {
  1: i32 field_1;
  2: binary field_2;
}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef Foo AdaptedFoo

struct Bar {
  10: list<binary> field_3;
  20: Foo field_4;
}

struct BoolList {
  1: list<bool> bools;
}

@cpp.EnableCustomTypeOrdering
struct MyStruct {
  1: optional Foo foo;
  2: optional list<Foo> foo_vector;
  3: optional set<Foo> foo_set;
  4: optional map<Foo, i32> foo_key_map;
  5: optional map<i32, Foo> foo_value_map;
  6: optional AdaptedFoo foo_adapted;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  7: optional list<Foo> foo_vector_adapted;
  8: optional list<AdaptedFoo> foo_adapted_vector;
  @cpp.Type{template = "std::unordered_map"}
  9: optional map<i32, Foo> foo_value_cpp_map;
}

struct EmptyIOBufPtr {
  @cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
  1: binary data;
}
