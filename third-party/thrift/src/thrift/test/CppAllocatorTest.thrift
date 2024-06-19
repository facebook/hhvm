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

namespace cpp2 apache.thrift.test

cpp_include "thrift/test/CppAllocatorTest.h"

struct AlwaysThrowChild {
  1: list_i32_1528 aa_list;
  2: set_i32_2716 aa_set;
  3: map_i32_i32_4816 aa_map;
  4: string_4597 aa_string;
  5: i32 not_a_container;
  6: list<i32> not_aa_list;
  7: set<i32> not_aa_set;
  8: map<i32, i32> not_aa_map;
  9: string not_aa_string;
} (cpp.allocator = "::ScopedAlwaysThrowAlloc<>")

struct AlwaysThrowParent {
  1: AlwaysThrowChild_4275 child;
} (cpp.allocator = "::ScopedAlwaysThrowAlloc<>")

struct ChildPmr {
  1: list_i32_8699 aa_list;
  2: set_i32_4098 aa_set;
  3: map_i32_i32_7355 aa_map;
  4: string_8090 aa_string;
  5: i32 not_a_container;
  6: list<i32> not_aa_list;
  7: set<i32> not_aa_set;
  8: map<i32, i32> not_aa_map;
  9: string not_aa_string;
} (cpp.allocator = "PmrByteAlloc")

struct ParentPmr {
  1: ChildPmr_7470 child;
  2: list_ChildPmr_9632 aa_child_list;
  3: list<ChildPmr> not_aa_child_list;
} (cpp.allocator = "PmrByteAlloc")

struct AAStruct {
  1: i32 foo;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

struct NoAllocatorVia {
  1: AAStruct_7495 foo;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

struct YesAllocatorVia {
  1: AAStruct_7495 foo (cpp.name = "bar");
} (cpp.allocator = "::ScopedStatefulAlloc<>", cpp.allocator_via = "bar")

struct AAStructPmr {
  1: i32 foo;
} (cpp.allocator = "PmrByteAlloc")

struct NoAllocatorViaPmr {
  1: AAStructPmr_1852 foo;
} (cpp.allocator = "PmrByteAlloc")

struct YesAllocatorViaPmr {
  1: AAStructPmr_1852 foo (cpp.name = "bar");
} (cpp.allocator = "PmrByteAlloc", cpp.allocator_via = "bar")

struct HasContainerFields {
  1: list_i32_4073 aa_list;
  2: set_i32_8876 aa_set;
  3: map_i32_i32_3938 aa_map;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

struct HasContainerFieldsPmr {
  1: list_i32_8699 aa_list;
  2: set_i32_4098 aa_set;
  3: map_i32_i32_7355 aa_map;
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
  1: set_i32_992 aa_set;
  2: map_i32_i32_4068 aa_map;
} (cpp.allocator = "::ScopedStatefulAlloc<>")

struct CountingChild {
  1: list_i32_8474 aa_list;
  2: set_i32_9206 aa_set;
  3: map_i32_i32_1496 aa_map;
  4: string_186 aa_string;
  5: i32 not_a_container;
  6: list<i32> not_aa_list;
  7: set<i32> not_aa_set;
  8: map<i32, i32> not_aa_map;
  9: string not_aa_string;
} (cpp.allocator = "::ScopedCountingAlloc<>")

struct CountingParent {
  1: list_CountingChild_2391 aa_child_list;
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
  @cpp.Ref{type = cpp.RefType.Unique}
  1: AlwaysThrowCppRefChild_9703 uniqueChild (
    cpp.template = "::AlwaysThrowUniquePtr",
  );
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: AlwaysThrowCppRefChild_9703 sharedChild;
  3: i32 no_alloc;
} (cpp.allocator = "::ScopedAlwaysThrowAlloc<>")

struct CountingCppRefChild {
  1: i32 value1;
  2: i64 value2;
} (cpp.allocator = "::ScopedCountingAlloc<>")

struct CountingCppRefParent {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: CountingCppRefChild_4892 uniqueChild (
    cpp.template = "::CountingUniquePtr",
  );
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: CountingCppRefChild_4892 sharedChild;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: CountingCppRefChild noAllocUniqueChild;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  4: CountingCppRefChild noAllocSharedChild;
  5: i32 no_alloc;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  6: list_i32_8474 allocVector;
  @thrift.Box
  7: optional CountingCppRefChild noAllocBoxedChild;
} (cpp.allocator = "::ScopedCountingAlloc<>")

// The following were automatically generated and may benefit from renaming.
typedef AAStructPmr (cpp.use_allocator = "1") AAStructPmr_1852
typedef AAStruct (cpp.use_allocator = "1") AAStruct_7495
typedef AlwaysThrowChild (cpp.use_allocator = "1") AlwaysThrowChild_4275
typedef AlwaysThrowCppRefChild (
  cpp.use_allocator = "1",
) AlwaysThrowCppRefChild_9703
typedef ChildPmr (cpp.use_allocator = "1") ChildPmr_7470
typedef CountingCppRefChild (cpp.use_allocator = "1") CountingCppRefChild_4892
typedef list<ChildPmr> (
  cpp.template = "std::pmr::vector",
  cpp.use_allocator = "1",
) list_ChildPmr_9632
typedef list<CountingChild> (
  cpp.template = "::CountingVector",
  cpp.use_allocator = "1",
) list_CountingChild_2391
typedef list<i32> (
  cpp.template = "::AlwaysThrowVector",
  cpp.use_allocator = "1",
) list_i32_1528
typedef list<i32> (
  cpp.template = "::StatefulAllocVector",
  cpp.use_allocator = "1",
) list_i32_4073
typedef list<i32> (
  cpp.template = "::CountingVector",
  cpp.use_allocator = "1",
) list_i32_8474
typedef list<i32> (
  cpp.template = "std::pmr::vector",
  cpp.use_allocator = "1",
) list_i32_8699
typedef map<i32, i32> (
  cpp.template = "::CountingMap",
  cpp.use_allocator = "1",
) map_i32_i32_1496
typedef map<i32, i32> (
  cpp.template = "::StatefulAllocMap",
  cpp.use_allocator = "1",
) map_i32_i32_3938
typedef map<i32, i32> (
  cpp.template = "::StatefulAllocSortedVectorMap",
  cpp.use_allocator = "1",
) map_i32_i32_4068
typedef map<i32, i32> (
  cpp.template = "::AlwaysThrowMap",
  cpp.use_allocator = "1",
) map_i32_i32_4816
typedef map<i32, i32> (
  cpp.template = "std::pmr::map",
  cpp.use_allocator = "1",
) map_i32_i32_7355
typedef set<i32> (
  cpp.template = "::AlwaysThrowSet",
  cpp.use_allocator = "1",
) set_i32_2716
typedef set<i32> (
  cpp.template = "std::pmr::set",
  cpp.use_allocator = "1",
) set_i32_4098
typedef set<i32> (
  cpp.template = "::StatefulAllocSet",
  cpp.use_allocator = "1",
) set_i32_8876
typedef set<i32> (
  cpp.template = "::CountingSet",
  cpp.use_allocator = "1",
) set_i32_9206
typedef set<i32> (
  cpp.template = "::StatefulAllocSortedVectorSet",
  cpp.use_allocator = "1",
) set_i32_992
typedef string (
  cpp.type = "::CountingString",
  cpp.use_allocator = "1",
) string_186
typedef string (
  cpp.type = "::AlwaysThrowString",
  cpp.use_allocator = "1",
) string_4597
typedef string (
  cpp.type = "std::pmr::string",
  cpp.use_allocator = "1",
) string_8090
