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

include "thrift/test/reflection/fatal_reflection_indirection.thrift"
include "thrift/annotation/cpp.thrift"

package "apache.org/thrift/reflection"

struct struct_with_included_indirections {
  1: i32 real;
  2: fatal_reflection_indirection.FakeI32 fake;
  3: fatal_reflection_indirection.HasANumber number;
  4: fatal_reflection_indirection.HasAResult result;

  // XXX: string type can not be included with cpp.indirection
  @cpp.Type{name = "::reflection_indirection::CppHasAPhrase"}
  5: string (cpp.indirection) phrase;
}
