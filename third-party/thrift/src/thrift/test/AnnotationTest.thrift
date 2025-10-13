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

cpp_include "<list>"

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@cpp.Type{template = "std::list"}
typedef list<i32> int_linked_list

@thrift.DeprecatedUnvalidatedAnnotations{
  items = {
    "cpp.methods": "public: void manuallyDefinedDummyMethod() {}",
    "java.final": "",
    "python.type": "DenseFoo",
  },
}
struct foo {
  1: i32 bar;
  2: i32 baz;
  3: i32 qux;
  4: i32 bop;
}

@thrift.DeprecatedUnvalidatedAnnotations{items = {"unicode.encoding": "UTF-16"}}
typedef string non_latin_string
typedef list<double_7102> tiny_float_list

// The following were automatically generated and may benefit from renaming.
@thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.fixed_point": "16"}}
typedef double double_7102
