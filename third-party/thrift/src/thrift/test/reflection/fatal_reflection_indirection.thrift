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

cpp_include "thrift/test/reflection/fatal_reflection_indirection_types.h"

package "apache.org/thrift/reflection"

namespace cpp2 reflection_indirection

include "thrift/annotation/cpp.thrift"

@cpp.Type{name = "reflection_indirection::CppFakeI32"}
typedef i32 FakeI32
@cpp.Type{name = "reflection_indirection::CppHasANumber"}
typedef i32 (cpp.indirection) HasANumber
@cpp.Type{name = "reflection_indirection::CppHasAResult"}
typedef i32 (cpp.indirection) HasAResult
@cpp.Type{name = "reflection_indirection::CppHasAPhrase"}
typedef string (cpp.indirection) HasAPhrase

struct struct_with_indirections {
  1: i32 real;
  2: FakeI32 fake;
  3: HasANumber number;
  4: HasAResult result;
  5: HasAPhrase phrase;
}
