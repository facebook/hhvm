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

#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>

#include <gtest/gtest.h>

// getDefaultAllocator() captures its backing resource on the FIRST call
// process-wide. This binary exercises the "custom resource installed before
// first capture" path: a resource set via setDefaultParserMemoryResource()
// must back the default allocator and receive its (de)allocations. Because the
// capture latches forever, this is mutually exclusive with the default-resource
// cases, which live in their own single-purpose binary.

#if FOLLY_HAS_MEMORY_RESOURCE

using apache::thrift::rocket::detail::getDefaultAllocator;
using apache::thrift::rocket::detail::setDefaultParserMemoryResource;

namespace {
// Minimal thread-safe memory_resource that delegates to new_delete_resource and
// counts (de)allocations, so tests can prove the default allocator routes
// through the installed resource.
class CountingResource : public std::pmr::memory_resource {
 public:
  std::atomic<size_t> allocateCount{0};
  std::atomic<size_t> deallocateCount{0};

 private:
  void* do_allocate(std::size_t bytes, std::size_t alignment) override {
    ++allocateCount;
    return std::pmr::new_delete_resource()->allocate(bytes, alignment);
  }

  void do_deallocate(
      void* p, std::size_t bytes, std::size_t alignment) override {
    if (p != nullptr) {
      ++deallocateCount;
      std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
    }
  }

  bool do_is_equal(
      const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }
};

// Process-lifetime instance. getDefaultAllocator() captures this pointer on
// first use and the process-wide static allocator may deallocate through it for
// the remainder of the process, so the resource must outlive that static --
// hence a leaked (never-destroyed) singleton rather than a stack local.
CountingResource& countingResource() {
  static auto* instance = new CountingResource();
  return *instance;
}
} // namespace

// Single test case by design: setDefaultParserMemoryResource() only takes
// effect before getDefaultAllocator() captures the resource on first use, after
// which the capture latches for the lifetime of the process. The install, the
// .resource() identity check, and the allocation exercise must therefore all
// run in one case, before any other code in this binary materializes the
// default allocator.
TEST(ParserCustomMemoryResourceTest, InstalledResourceHonoredAndUsed) {
  CountingResource& counting = countingResource();

  // Must be installed before any getDefaultAllocator() use in this process.
  setDefaultParserMemoryResource(&counting);

  auto& allocator = getDefaultAllocator();
  ASSERT_EQ(allocator.resource(), &counting);

  const size_t allocsBefore = counting.allocateCount.load();
  const size_t deallocsBefore = counting.deallocateCount.load();

  // polymorphic_allocator<uint8_t>::allocate(n) allocates n bytes; this proves
  // the default allocator routes (de)allocations through the installed
  // resource.
  auto* p = allocator.allocate(64);
  ASSERT_NE(p, nullptr);
  if (p != nullptr) {
    allocator.deallocate(p, 64);
  }

  EXPECT_EQ(counting.allocateCount.load(), allocsBefore + 1);
  EXPECT_EQ(counting.deallocateCount.load(), deallocsBefore + 1);
}

#endif // FOLLY_HAS_MEMORY_RESOURCE
