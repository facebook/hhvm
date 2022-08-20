/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 reflection_indirection

typedef i32 (cpp.type = 'CppFakeI32') FakeI32
typedef i32 (cpp.type = 'CppHasANumber', cpp.indirection) HasANumber
typedef i32 (cpp.type = 'CppHasAResult', cpp.indirection) HasAResult
typedef string (cpp.type = 'CppHasAPhrase', cpp.indirection) HasAPhrase

struct struct_with_indirections {
  1: i32 real;
  2: FakeI32 fake;
  3: HasANumber number;
  4: HasAResult result;
  5: HasAPhrase phrase;
}
