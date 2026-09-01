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

package "meta.com/thrift/test/benchmark/allocator_emplace"

namespace cpp2 thrift.benchmark.allocator_emplace

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

cpp_include "<memory_resource>"

@thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.use_allocator": "1"}}
@cpp.Type{name = "std::pmr::string"}
typedef string PmrString

@thrift.DeprecatedUnvalidatedAnnotations{items = {"cpp.use_allocator": "1"}}
@cpp.Type{template = "std::pmr::vector"}
typedef list<PmrString> PmrStringList

// An allocator-aware struct with an allocator-aware list element: the shape
// that reaches emplace_back_default. `names` is a plain list<string> on the
// wire, so it deserializes from the same bytes as an unannotated struct.
@thrift.DeprecatedUnvalidatedAnnotations{
  items = {"cpp.allocator": "std::pmr::polymorphic_allocator<std::byte>"},
}
struct AllocatorAwareNames {
  1: PmrStringList names;
}

// Same wire layout, plain std types: the baseline the annotated struct is
// measured against.
struct Names {
  1: list<string> names;
}
