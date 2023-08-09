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

#include <thrift/test/CppAllocatorTest.h>

#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/CppAllocatorTest_types.h>

#include <folly/portability/GTest.h>

namespace apache::thrift::test {

static const char* kTooLong =
    "This is too long for the small string optimization";

// Workaround an ambiguity when comparing allocators with different value types
// which was introduced in C++20.
template <typename T>
static ScopedStatefulAlloc<> get_allocator(const T& container) {
  return ScopedStatefulAlloc<>(container.get_allocator());
}

TEST(CppAllocatorTest, AlwaysThrowAllocator) {
  ScopedAlwaysThrowAlloc<> alloc;
  AlwaysThrowParent s(alloc);

  EXPECT_THROW(s.child()->aa_list()->emplace_back(42), std::bad_alloc);
  EXPECT_THROW(s.child()->aa_set()->emplace(42), std::bad_alloc);
  EXPECT_THROW(s.child()->aa_map()->emplace(42, 42), std::bad_alloc);
  EXPECT_THROW(s.child()->aa_string()->assign(kTooLong), std::bad_alloc);

  EXPECT_NO_THROW(s.child()->not_aa_list()->emplace_back(42));
  EXPECT_NO_THROW(s.child()->not_aa_set()->emplace(42));
  EXPECT_NO_THROW(s.child()->not_aa_map()->emplace(42, 42));
  EXPECT_NO_THROW(s.child()->not_aa_string()->assign(kTooLong));
}

TEST(CppAllocatorTest, UsesAllocatorPmr) {
  PmrByteAlloc alloc(std::pmr::null_memory_resource());
  ParentPmr s(alloc);

  EXPECT_THROW(s.child()->aa_list()->emplace_back(42), std::bad_alloc);
  EXPECT_THROW(s.child()->aa_set()->emplace(42), std::bad_alloc);
  EXPECT_THROW(s.child()->aa_map()->emplace(42, 42), std::bad_alloc);
  EXPECT_THROW(s.child()->aa_string()->assign(kTooLong), std::bad_alloc);

  EXPECT_NO_THROW(s.child()->not_aa_list()->emplace_back(42));
  EXPECT_NO_THROW(s.child()->not_aa_set()->emplace(42));
  EXPECT_NO_THROW(s.child()->not_aa_map()->emplace(42, 42));
  EXPECT_NO_THROW(s.child()->not_aa_string()->assign(kTooLong));
}

TEST(CppAllocatorTest, GetAllocator) {
  ScopedStatefulAlloc<> alloc(42);

  NoAllocatorVia s1(alloc);
  EXPECT_EQ(alloc, s1.get_allocator());

  YesAllocatorVia s2(alloc);
  EXPECT_EQ(alloc, s2.get_allocator());
}

TEST(CppAllocatorTest, GetAllocatorPmr) {
  std::pmr::monotonic_buffer_resource res;
  PmrByteAlloc alloc(&res);

  NoAllocatorViaPmr s1(alloc);
  EXPECT_EQ(alloc, s1.get_allocator());

  YesAllocatorViaPmr s2(alloc);
  EXPECT_EQ(alloc, s2.get_allocator());
}

TEST(CppAllocatorTest, AllocatorVia) {
  NoAllocatorVia s1;
  YesAllocatorVia s2;
  EXPECT_GT(sizeof(s1), sizeof(s2));
}

TEST(CppAllocatorTest, AllocatorViaPmr) {
  NoAllocatorViaPmr s1;
  YesAllocatorViaPmr s2;
  EXPECT_GT(sizeof(s1), sizeof(s2));
}

TEST(CppAllocatorTest, Deserialize) {
  using serializer = apache::thrift::CompactSerializer;

  HasContainerFields s1;
  s1.aa_list() = {1, 2, 3};
  s1.aa_set() = {1, 2, 3};
  s1.aa_map() = {{1, 1}, {2, 2}, {3, 3}};

  auto str = serializer::serialize<std::string>(s1);

  ScopedStatefulAlloc<> alloc(42);
  HasContainerFields s2(alloc);
  EXPECT_EQ(s2.get_allocator(), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_list()), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_set()), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_map()), alloc);

  serializer::deserialize(str, s2);
  EXPECT_EQ(s2.aa_list(), (StatefulAllocVector<int32_t>{1, 2, 3}));
  EXPECT_EQ(s2.aa_set(), (StatefulAllocSet<int32_t>{1, 2, 3}));
  EXPECT_EQ(
      s2.aa_map(),
      (StatefulAllocMap<int32_t, int32_t>{{1, 1}, {2, 2}, {3, 3}}));

  EXPECT_EQ(s2.get_allocator(), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_list()), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_set()), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_map()), alloc);
}

TEST(CppAllocatorTest, DeserializePmr) {
  using serializer = apache::thrift::CompactSerializer;

  HasContainerFieldsPmr s1;
  s1.aa_list() = {1, 2, 3};
  s1.aa_set() = {1, 2, 3};
  s1.aa_map() = {{1, 1}, {2, 2}, {3, 3}};

  auto str = serializer::serialize<std::string>(s1);

  std::pmr::monotonic_buffer_resource res;
  PmrByteAlloc alloc(&res);

  HasContainerFieldsPmr s2(alloc);
  EXPECT_EQ(s2.get_allocator(), alloc);
  EXPECT_EQ(s2.aa_list()->get_allocator(), alloc);
  EXPECT_EQ(s2.aa_set()->get_allocator(), alloc);
  EXPECT_EQ(s2.aa_map()->get_allocator(), alloc);

  serializer::deserialize(str, s2);
  EXPECT_EQ(s2.aa_list(), (std::pmr::vector<int32_t>{1, 2, 3}));
  EXPECT_EQ(s2.aa_set(), (std::pmr::set<int32_t>{1, 2, 3}));
  EXPECT_EQ(
      s2.aa_map(), (std::pmr::map<int32_t, int32_t>{{1, 1}, {2, 2}, {3, 3}}));

  EXPECT_EQ(s2.get_allocator(), alloc);
  EXPECT_EQ(s2.aa_list()->get_allocator(), alloc);
  EXPECT_EQ(s2.aa_set()->get_allocator(), alloc);
  EXPECT_EQ(s2.aa_map()->get_allocator(), alloc);
}

TEST(CppAllocatorTest, UsesTypedef) {
  ScopedStatefulAlloc<> alloc(42);
  UsesTypedef s(alloc);
  EXPECT_EQ(alloc, s.get_allocator());
}

TEST(CppAllocatorTest, UsesTypedefPmr) {
  std::pmr::monotonic_buffer_resource res;
  PmrByteAlloc alloc(&res);
  UsesTypedefPmr s(alloc);
  EXPECT_EQ(alloc, s.get_allocator());
}

TEST(CppAllocatorTest, DeserializeNested) {
  using serializer = apache::thrift::CompactSerializer;

  HasNestedContainerFields s1;
  s1.aa_map_of_map() = {{42, {{42, 42}}}};
  s1.aa_map_of_set() = {{42, {42}}};

  auto str = serializer::serialize<std::string>(s1);

  ScopedStatefulAlloc<> alloc(42);
  HasNestedContainerFields s2(alloc);

  EXPECT_EQ(s2.get_allocator(), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_map_of_map()), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_map_of_set()), alloc);

  serializer::deserialize(str, s2);
  EXPECT_EQ(get_allocator(*s2.aa_map_of_map()), alloc);
  EXPECT_EQ(get_allocator(s2.aa_map_of_map()->at(42)), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_map_of_set()), alloc);
  EXPECT_EQ(get_allocator(s2.aa_map_of_set()->at(42)), alloc);
}

TEST(CppAllocatorTest, DeserializeNestedPmr) {
  using serializer = apache::thrift::CompactSerializer;

  HasNestedContainerFieldsPmr s1;
  s1.aa_map_of_map() = {{42, {{42, 42}}}};
  s1.aa_map_of_set() = {{42, {42}}};

  auto str = serializer::serialize<std::string>(s1);

  std::pmr::monotonic_buffer_resource res;
  PmrByteAlloc alloc(&res);

  HasNestedContainerFieldsPmr s2(alloc);

  EXPECT_EQ(s2.get_allocator(), alloc);
  EXPECT_EQ(s2.aa_map_of_map()->get_allocator(), alloc);
  EXPECT_EQ(s2.aa_map_of_set()->get_allocator(), alloc);

  serializer::deserialize(str, s2);
  EXPECT_EQ(s2.aa_map_of_map()->get_allocator(), alloc);
  EXPECT_EQ(s2.aa_map_of_map()->at(42).get_allocator(), alloc);
  EXPECT_EQ(s2.aa_map_of_set()->get_allocator(), alloc);
  EXPECT_EQ(s2.aa_map_of_set()->at(42).get_allocator(), alloc);
}

TEST(CppAllocatorTest, DeserializeSortedUniqueConstructible) {
  using serializer = apache::thrift::CompactSerializer;

  HasSortedUniqueConstructibleFields s1;
  s1.aa_set() = {1, 2, 3};
  s1.aa_map() = {{1, 1}, {2, 2}, {3, 3}};

  auto str = serializer::serialize<std::string>(s1);

  ScopedStatefulAlloc<> alloc(42);
  HasSortedUniqueConstructibleFields s2(alloc);
  EXPECT_EQ(s2.get_allocator(), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_set()), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_map()), alloc);

  serializer::deserialize(str, s2);
  EXPECT_EQ(s2.aa_set(), (StatefulAllocSortedVectorSet<int32_t>{1, 2, 3}));
  EXPECT_EQ(
      s2.aa_map(),
      (StatefulAllocSortedVectorMap<int32_t, int32_t>{{1, 1}, {2, 2}, {3, 3}}));

  EXPECT_EQ(s2.get_allocator(), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_set()), alloc);
  EXPECT_EQ(get_allocator(*s2.aa_map()), alloc);
}

#define EXPECT_ALLOC(a, x)      \
  {                             \
    auto c = a.getCount();      \
    x;                          \
    EXPECT_GT(a.getCount(), c); \
  }
#define EXPECT_NO_ALLOC(a, x)   \
  {                             \
    auto c = a.getCount();      \
    x;                          \
    EXPECT_EQ(a.getCount(), c); \
  }

TEST(CppAllocatorTest, CountingAllocator) {
  ScopedCountingAlloc<> alloc;
  CountingParent s(alloc);

  EXPECT_ALLOC(alloc, s.aa_child_list()->emplace_back());
  auto& aa_child = s.aa_child_list()[0];

  EXPECT_ALLOC(alloc, aa_child.aa_list()->emplace_back(42));
  EXPECT_ALLOC(alloc, aa_child.aa_set()->emplace(42));
  EXPECT_ALLOC(alloc, aa_child.aa_map()->emplace(42, 42));
  EXPECT_ALLOC(alloc, aa_child.aa_string()->assign(kTooLong));

  EXPECT_NO_ALLOC(alloc, aa_child.not_aa_list()->emplace_back(42));
  EXPECT_NO_ALLOC(alloc, aa_child.not_aa_set()->emplace(42));
  EXPECT_NO_ALLOC(alloc, aa_child.not_aa_map()->emplace(42, 42));
  EXPECT_NO_ALLOC(alloc, aa_child.not_aa_string()->assign(kTooLong));

  EXPECT_NO_ALLOC(alloc, s.not_aa_child_list()->emplace_back());
  auto& not_aa_child = s.not_aa_child_list()[0];

  EXPECT_NO_ALLOC(alloc, not_aa_child.aa_list()->emplace_back(42));
  EXPECT_NO_ALLOC(alloc, not_aa_child.aa_set()->emplace(42));
  EXPECT_NO_ALLOC(alloc, not_aa_child.aa_map()->emplace(42, 42));
  EXPECT_NO_ALLOC(alloc, not_aa_child.aa_string()->assign(kTooLong));

  EXPECT_NO_ALLOC(alloc, not_aa_child.not_aa_list()->emplace_back(42));
  EXPECT_NO_ALLOC(alloc, not_aa_child.not_aa_set()->emplace(42));
  EXPECT_NO_ALLOC(alloc, not_aa_child.not_aa_map()->emplace(42, 42));
  EXPECT_NO_ALLOC(alloc, not_aa_child.not_aa_string()->assign(kTooLong));
}

TEST(CppAllocatorTest, CountingResource) {
  CountingPmrResource res;
  PmrByteAlloc alloc(&res);
  ParentPmr s(alloc);

  EXPECT_ALLOC(res, s.aa_child_list()->emplace_back());
  auto& aa_child = s.aa_child_list()[0];

  EXPECT_ALLOC(res, aa_child.aa_list()->emplace_back(42));
  EXPECT_ALLOC(res, aa_child.aa_set()->emplace(42));
  EXPECT_ALLOC(res, aa_child.aa_map()->emplace(42, 42));
  EXPECT_ALLOC(res, aa_child.aa_string()->assign(kTooLong));

  EXPECT_NO_ALLOC(res, aa_child.not_aa_list()->emplace_back(42));
  EXPECT_NO_ALLOC(res, aa_child.not_aa_set()->emplace(42));
  EXPECT_NO_ALLOC(res, aa_child.not_aa_map()->emplace(42, 42));
  EXPECT_NO_ALLOC(res, aa_child.not_aa_string()->assign(kTooLong));

  EXPECT_NO_ALLOC(res, s.not_aa_child_list()->emplace_back());
  auto& not_aa_child = s.not_aa_child_list()[0];

  EXPECT_NO_ALLOC(res, not_aa_child.aa_list()->emplace_back(42));
  EXPECT_NO_ALLOC(res, not_aa_child.aa_set()->emplace(42));
  EXPECT_NO_ALLOC(res, not_aa_child.aa_map()->emplace(42, 42));
  EXPECT_NO_ALLOC(res, not_aa_child.aa_string()->assign(kTooLong));

  EXPECT_NO_ALLOC(res, not_aa_child.not_aa_list()->emplace_back(42));
  EXPECT_NO_ALLOC(res, not_aa_child.not_aa_set()->emplace(42));
  EXPECT_NO_ALLOC(res, not_aa_child.not_aa_map()->emplace(42, 42));
  EXPECT_NO_ALLOC(res, not_aa_child.not_aa_string()->assign(kTooLong));
}

TEST(CppAllocatorTest, PropagateAllAlloc) {
  const PropagateAllAlloc alloc1(1);
  const PropagateAllAlloc alloc2(2);

  {
    const PropagateAllAllocStruct src(alloc1);
    PropagateAllAllocStruct dst(alloc2);
    dst = src;
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }

  {
    PropagateAllAllocStruct src(alloc1);
    PropagateAllAllocStruct dst(alloc2);
    dst = std::move(src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }

  {
    PropagateAllAllocStruct src(alloc1);
    PropagateAllAllocStruct dst(alloc2);
    swap(dst, src);
    EXPECT_EQ(src.get_allocator(), alloc2);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }
}

TEST(CppAllocatorTest, PropagateNoneAlloc) {
  const PropagateNoneAlloc alloc1(1);
  const PropagateNoneAlloc alloc2(2);

  {
    const PropagateNoneAllocStruct src(alloc1);
    PropagateNoneAllocStruct dst(alloc2);
    dst = src;
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }

  {
    PropagateNoneAllocStruct src(alloc1);
    PropagateNoneAllocStruct dst(alloc2);
    dst = std::move(src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }

  {
    PropagateNoneAllocStruct src(alloc1);
    PropagateNoneAllocStruct dst(alloc2);
    swap(dst, src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }
}

TEST(CppAllocatorTest, PropagateOnlyCopyAlloc) {
  const PropagateOnlyCopyAlloc alloc1(1);
  const PropagateOnlyCopyAlloc alloc2(2);

  {
    const PropagateOnlyCopyAllocStruct src(alloc1);
    PropagateOnlyCopyAllocStruct dst(alloc2);
    dst = src;
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }

  {
    PropagateOnlyCopyAllocStruct src(alloc1);
    PropagateOnlyCopyAllocStruct dst(alloc2);
    dst = std::move(src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }

  {
    PropagateOnlyCopyAllocStruct src(alloc1);
    PropagateOnlyCopyAllocStruct dst(alloc2);
    swap(dst, src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }
}

TEST(CppAllocatorTest, PropagateOnlyMoveAlloc) {
  const PropagateOnlyMoveAlloc alloc1(1);
  const PropagateOnlyMoveAlloc alloc2(2);

  {
    const PropagateOnlyMoveAllocStruct src(alloc1);
    PropagateOnlyMoveAllocStruct dst(alloc2);
    dst = src;
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }

  {
    PropagateOnlyMoveAllocStruct src(alloc1);
    PropagateOnlyMoveAllocStruct dst(alloc2);
    dst = std::move(src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }

  {
    PropagateOnlyMoveAllocStruct src(alloc1);
    PropagateOnlyMoveAllocStruct dst(alloc2);
    swap(dst, src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }
}

TEST(CppAllocatorTest, PropagateOnlySwapAlloc) {
  const PropagateOnlySwapAlloc alloc1(1);
  const PropagateOnlySwapAlloc alloc2(2);

  {
    const PropagateOnlySwapAllocStruct src(alloc1);
    PropagateOnlySwapAllocStruct dst(alloc2);
    dst = src;
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }

  {
    PropagateOnlySwapAllocStruct src(alloc1);
    PropagateOnlySwapAllocStruct dst(alloc2);
    dst = std::move(src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }

  {
    PropagateOnlySwapAllocStruct src(alloc1);
    PropagateOnlySwapAllocStruct dst(alloc2);
    swap(dst, src);
    EXPECT_EQ(src.get_allocator(), alloc2);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }
}

TEST(CppAllocatorTest, PropagateCopyMoveAlloc) {
  const PropagateCopyMoveAlloc alloc1(1);
  const PropagateCopyMoveAlloc alloc2(2);

  {
    const PropagateCopyMoveAllocStruct src(alloc1);
    PropagateCopyMoveAllocStruct dst(alloc2);
    dst = src;
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }

  {
    PropagateCopyMoveAllocStruct src(alloc1);
    PropagateCopyMoveAllocStruct dst(alloc2);
    dst = std::move(src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }

  {
    PropagateCopyMoveAllocStruct src(alloc1);
    PropagateCopyMoveAllocStruct dst(alloc2);
    swap(dst, src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }
}

TEST(CppAllocatorTest, PropagateCopySwapAlloc) {
  const PropagateCopySwapAlloc alloc1(1);
  const PropagateCopySwapAlloc alloc2(2);

  {
    const PropagateCopySwapAllocStruct src(alloc1);
    PropagateCopySwapAllocStruct dst(alloc2);
    dst = src;
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }

  {
    PropagateCopySwapAllocStruct src(alloc1);
    PropagateCopySwapAllocStruct dst(alloc2);
    dst = std::move(src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }

  {
    PropagateCopySwapAllocStruct src(alloc1);
    PropagateCopySwapAllocStruct dst(alloc2);
    swap(dst, src);
    EXPECT_EQ(src.get_allocator(), alloc2);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }
}

TEST(CppAllocatorTest, PropagateMoveSwapAlloc) {
  const PropagateMoveSwapAlloc alloc1(1);
  const PropagateMoveSwapAlloc alloc2(2);

  {
    const PropagateMoveSwapAllocStruct src(alloc1);
    PropagateMoveSwapAllocStruct dst(alloc2);
    dst = src;
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc2);
  }

  {
    PropagateMoveSwapAllocStruct src(alloc1);
    PropagateMoveSwapAllocStruct dst(alloc2);
    dst = std::move(src);
    EXPECT_EQ(src.get_allocator(), alloc1);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }

  {
    PropagateMoveSwapAllocStruct src(alloc1);
    PropagateMoveSwapAllocStruct dst(alloc2);
    swap(dst, src);
    EXPECT_EQ(src.get_allocator(), alloc2);
    EXPECT_EQ(dst.get_allocator(), alloc1);
  }
}

TEST(CppAllocatorTest, AlwaysThrowAllocatorCppRef) {
  ScopedAlwaysThrowAlloc<> alloc;
  // this is just death instead of a throw since it is throwing an exception on
  // constuctor somehow which results in calling std::terminate
  EXPECT_DEATH({ AlwaysThrowCppRefParent parent(alloc); }, "");
}

TEST(CppAllocatorTest, AlwaysThrowAllocatorCppRefCount) {
  ScopedCountingAlloc<> alloc;
  EXPECT_ALLOC(alloc, { CountingCppRefParent parent(alloc); });
  CountingCppRefParent parent(alloc);
  // check propagation of allocator for containers with shared_ptr
  EXPECT_ALLOC(alloc, parent.allocVector_ref()->emplace_back(1));
  op::getValueOrNull(op::get<ident::uniqueChild>(parent))->value1() = 10;
  EXPECT_EQ(parent.uniqueChild()->value1(), 10);
}

TEST(CppAllocatorTest, CopyConstructorPropagatesAllocator) {
  ScopedStatefulAlloc<> alloc(42);
  AAStruct s1(alloc);
  AAStruct s2(s1);
  EXPECT_EQ(alloc, s1.get_allocator());
  EXPECT_EQ(alloc, s2.get_allocator());
}

TEST(CppAllocatorTest, MoveConstructorPropagatesAllocator) {
  ScopedStatefulAlloc<> alloc(42);
  AAStruct s1(alloc);
  AAStruct s2(std::move(s1));
  EXPECT_EQ(s1.get_allocator(), alloc);
  EXPECT_EQ(s2.get_allocator(), alloc);
}

TEST(CppAllocatorTest, DefaultConstructor1allocator) {
  CountingParent s;
  ScopedCountingAlloc<> alloc = s.get_allocator();
  EXPECT_ALLOC(alloc, s.aa_child_list()->emplace_back());
  auto& child = s.aa_child_list()[0];
  EXPECT_ALLOC(alloc, child.aa_list()->emplace_back(42));
  child.aa_string() = "abcdefg"; // Regardless of short strings optimization
  EXPECT_EQ(alloc, ScopedCountingAlloc<>(s.aa_child_list()->get_allocator()));
  EXPECT_EQ(alloc, ScopedCountingAlloc<>(child.aa_list()->get_allocator()));
  EXPECT_EQ(alloc, ScopedCountingAlloc<>(child.aa_string()->get_allocator()));
}

} // namespace apache::thrift::test
