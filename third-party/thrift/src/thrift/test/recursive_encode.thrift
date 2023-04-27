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
cpp_include "thrift/test/AdapterTest.h"
cpp_include "<list>"

package "apache.org/thrift/test"

struct Foo {
  1: i32 field;
}

typedef list<Foo> (cpp.type = "std::list<::apache::thrift::test::Foo>") FooList

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
struct Bar {
  1: i32 field;
  2: FooList foos;
}

struct Baz {
  1: i32 field;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  2: Bar bar;
}
