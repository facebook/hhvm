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

cpp_include "<list>"

typedef list<i32> (cpp.template = "std::list") int_linked_list

struct foo {
  1: i32 bar;
  2: i32 baz;
  3: i32 qux;
  4: i32 bop;
} (
  cpp.type = "DenseFoo",
  python.type = "DenseFoo",
  java.final = "",
  cpp2.methods = "public: void manuallyDefinedDummyMethod() {}",
)

typedef string (unicode.encoding = "UTF-16") non_latin_string
typedef list<double_7102> tiny_float_list

// The following were automatically generated and may benefit from renaming.
typedef double (cpp.fixed_point = "16") double_7102

@cpp.RuntimeAnnotation
@scope.Field
struct Oncall {
  1: string name;
}

@cpp.RuntimeAnnotation
@scope.Struct
@scope.Field
struct Doc {
  1: string text;
}

@cpp.RuntimeAnnotation
@scope.Field
struct Sensitive {}

@scope.Field
struct Other {}

@Doc{text = "I am a struct"}
struct MyStruct {
  @Oncall{name = "thrift"}
  @Sensitive
  @Other
  1: string field;
}
