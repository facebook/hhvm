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

namespace cpp2 apache.thrift.test

cpp_include "thrift/test/CppAllocatorTest.h"

struct AlwaysThrowChild {
  1: list<i32> (
    cpp.use_allocator,
    cpp.template = "::AlwaysThrowVector",
  ) aa_list;
  2: set<i32> (cpp.use_allocator, cpp.template = "::AlwaysThrowSet") aa_set;
  3: map<i32, i32> (
    cpp.use_allocator,
    cpp.template = "::AlwaysThrowMap",
  ) aa_map;
  4: string (cpp.use_allocator, cpp.type = "::AlwaysThrowString") aa_string;
  5: i32 not_a_container;
  6: list<i32> not_aa_list;
  7: set<i32> not_aa_set;
  8: map<i32, i32> not_aa_map;
  9: string not_aa_string;
} (cpp.allocator = "::ScopedAlwaysThrowAlloc<>")

struct AlwaysThrowParent {
  1: AlwaysThrowChild (cpp.use_allocator) child;
} (cpp.allocator = "::ScopedAlwaysThrowAlloc<>")

struct ChildPmr {
  1: list<i32> (cpp.use_allocator, cpp.template = "std::pmr::vector") aa_list;
  2: set<i32> (cpp.use_allocator, cpp.template = "std::pmr::set") aa_set;
  3: map<i32, i32> (cpp.use_allocator, cpp.template = "std::pmr::map") aa_map;
  4: string (cpp.use_allocator, cpp.type = "std::pmr::string") aa_string;
  5: i32 not_a_container;
  6: list<i32> not_aa_list;
  7: set<i32> not_aa_set;
  8: map<i32, i32> not_aa_map;
  9: string not_aa_string;
} (cpp.allocator = "PmrByteAlloc")

struct ParentPmr {
  1: ChildPmr (cpp.use_allocator) child;
  2: list<ChildPmr> (
    cpp.use_allocator,
    cpp.template = "std::pmr::vector",
  ) aa_child_list;
  3: list<ChildPmr> not_aa_child_list;
} (cpp.allocator = "PmrByteAlloc")

struct AAStruct {
  1: i32 foo;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

struct NoAllocatorVia {
  1: AAStruct (cpp.use_allocator) foo;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

struct YesAllocatorVia {
  1: AAStruct (cpp.use_allocator) foo (cpp.name = "bar");
} (cpp.allocator = "::ScopedStatefulAlloc<>", cpp.allocator_via = "bar")

struct AAStructPmr {
  1: i32 foo;
} (cpp.allocator = "PmrByteAlloc")

struct NoAllocatorViaPmr {
  1: AAStructPmr (cpp.use_allocator) foo;
} (cpp.allocator = "PmrByteAlloc")

struct YesAllocatorViaPmr {
  1: AAStructPmr (cpp.use_allocator) foo (cpp.name = "bar");
} (cpp.allocator = "PmrByteAlloc", cpp.allocator_via = "bar")

struct HasContainerFields {
  1: list<i32> (
    cpp.use_allocator,
    cpp.template = "::StatefulAllocVector",
  ) aa_list;
  2: set<i32> (cpp.use_allocator, cpp.template = "::StatefulAllocSet") aa_set;
  3: map<i32, i32> (
    cpp.use_allocator,
    cpp.template = "::StatefulAllocMap",
  ) aa_map;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

struct HasContainerFieldsPmr {
  1: list<i32> (cpp.use_allocator, cpp.template = "std::pmr::vector") aa_list;
  2: set<i32> (cpp.use_allocator, cpp.template = "std::pmr::set") aa_set;
  3: map<i32, i32> (cpp.use_allocator, cpp.template = "std::pmr::map") aa_map;
} (cpp.allocator = "PmrByteAlloc")

typedef map<i32, i32> (
  cpp.use_allocator,
  cpp.template = "::StatefulAllocMap",
) StatefulAllocIntMap

typedef map<i32, StatefulAllocIntMap> (
  cpp.use_allocator,
  cpp.template = "::StatefulAllocMap",
) StatefulAllocMapMap

typedef set<i32> (
  cpp.use_allocator,
  cpp.template = "::StatefulAllocSet",
) StatefulAllocIntSet

typedef map<i32, StatefulAllocIntSet> (
  cpp.use_allocator,
  cpp.template = "::StatefulAllocMap",
) StatefulAllocMapSet

struct UsesTypedef {
  1: StatefulAllocIntMap aa_map;
} (cpp.allocator = "::ScopedStatefulAlloc<>", cpp.allocator_via = "aa_map")

struct HasNestedContainerFields {
  1: StatefulAllocMapMap aa_map_of_map;
  2: StatefulAllocMapSet aa_map_of_set;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

typedef map<i32, i32> (
  cpp.use_allocator,
  cpp.template = "std::pmr::map",
) PmrIntMap

struct UsesTypedefPmr {
  1: PmrIntMap aa_map;
} (cpp.allocator = "PmrByteAlloc", cpp.allocator_via = "aa_map")

typedef map<i32, PmrIntMap> (
  cpp.use_allocator,
  cpp.template = "std::pmr::map",
) PmrMapMap

typedef set<i32> (cpp.use_allocator, cpp.template = "std::pmr::set") PmrIntSet

typedef map<i32, PmrIntSet> (
  cpp.use_allocator,
  cpp.template = "std::pmr::map",
) PmrMapSet

struct HasNestedContainerFieldsPmr {
  1: PmrMapMap aa_map_of_map;
  2: PmrMapSet aa_map_of_set;
} (cpp.allocator = "PmrByteAlloc")

struct HasSortedUniqueConstructibleFields {
  1: set<i32> (
    cpp.use_allocator,
    cpp.template = "::StatefulAllocSortedVectorSet",
  ) aa_set;
  2: map<i32, i32> (
    cpp.use_allocator,
    cpp.template = "::StatefulAllocSortedVectorMap",
  ) aa_map;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

struct CountingChild {
  1: list<i32> (cpp.use_allocator, cpp.template = "::CountingVector") aa_list;
  2: set<i32> (cpp.use_allocator, cpp.template = "::CountingSet") aa_set;
  3: map<i32, i32> (cpp.use_allocator, cpp.template = "::CountingMap") aa_map;
  4: string (cpp.use_allocator, cpp.type = "::CountingString") aa_string;
  5: i32 not_a_container;
  6: list<i32> not_aa_list;
  7: set<i32> not_aa_set;
  8: map<i32, i32> not_aa_map;
  9: string not_aa_string;
} (cpp.allocator = "::ScopedCountingAlloc<>")

struct CountingParent {
  1: list<CountingChild> (
    cpp.use_allocator,
    cpp.template = "::CountingVector",
  ) aa_child_list;
  2: list<CountingChild> not_aa_child_list;
} (cpp.allocator = "::ScopedCountingAlloc<>")

struct PropagateAllAllocStruct {
  1: i32 foo;
} (cpp.allocator = "::PropagateAllAlloc")

struct PropagateNoneAllocStruct {
  1: i32 foo;
} (cpp.allocator = "::PropagateNoneAlloc")

struct PropagateOnlyCopyAllocStruct {
  1: i32 foo;
} (cpp.allocator = "::PropagateOnlyCopyAlloc")

struct PropagateOnlyMoveAllocStruct {
  1: i32 foo;
} (cpp.allocator = "::PropagateOnlyMoveAlloc")

struct PropagateOnlySwapAllocStruct {
  1: i32 foo;
} (cpp.allocator = "::PropagateOnlySwapAlloc")

struct PropagateCopyMoveAllocStruct {
  1: i32 foo;
} (cpp.allocator = "::PropagateCopyMoveAlloc")

struct PropagateCopySwapAllocStruct {
  1: i32 foo;
} (cpp.allocator = "::PropagateCopySwapAlloc")

struct PropagateMoveSwapAllocStruct {
  1: i32 foo;
} (cpp.allocator = "::PropagateMoveSwapAlloc")

struct AlwaysThrowCppRefChild {
  1: i32 value1;
  2: i64 value2;
} (cpp.allocator = "::ScopedAlwaysThrowAlloc<>")

struct AlwaysThrowCppRefParent {
  1: AlwaysThrowCppRefChild (cpp.use_allocator) uniqueChild (
    cpp.ref_type = "unique",
    cpp.template = "::AlwaysThrowUniquePtr",
  );
  2: AlwaysThrowCppRefChild (cpp.use_allocator) sharedChild (
    cpp.ref_type = "shared",
  );
  3: i32 no_alloc;
} (cpp.allocator = "::ScopedAlwaysThrowAlloc<>")

struct CountingCppRefChild {
  1: i32 value1;
  2: i64 value2;
} (cpp.allocator = "::ScopedCountingAlloc<>")

struct CountingCppRefParent {
  1: CountingCppRefChild (cpp.use_allocator) uniqueChild (
    cpp.ref_type = "unique",
    cpp.template = "::CountingUniquePtr",
  );
  2: CountingCppRefChild (cpp.use_allocator) sharedChild (
    cpp.ref_type = "shared",
  );
  3: CountingCppRefChild noAllocUniqueChild (cpp.ref_type = "unique");
  4: CountingCppRefChild noAllocSharedChild (cpp.ref_type = "shared");
  5: i32 no_alloc;
  6: list<i32> (
    cpp.use_allocator,
    cpp.template = "::CountingVector",
  ) allocVector (cpp.ref_type = "shared");
} (cpp.allocator = "::ScopedCountingAlloc<>")
