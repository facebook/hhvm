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
// process-wide and latches a "captured" flag forever after. This binary
// exercises the "no resource installed" path: the very first capture must fall
// back to std::pmr::get_default_resource(), and a later setter call must FATAL.
// These cases are order-dependent and cannot coexist with the "custom resource
// installed first" cases, which live in their own single-purpose binary.

#if FOLLY_HAS_MEMORY_RESOURCE

using apache::thrift::rocket::detail::getDefaultAllocator;
using apache::thrift::rocket::detail::setDefaultParserMemoryResource;

TEST(ParserDefaultMemoryResourceTest, DefaultAllocatorUsesStdDefaultResource) {
  // First getDefaultAllocator() use in this process, with no resource installed
  // -> captures std::pmr::get_default_resource().
  EXPECT_EQ(getDefaultAllocator().resource(), std::pmr::get_default_resource());
}

TEST(ParserDefaultMemoryResourceTest, GetDefaultAllocatorIsStable) {
  auto& first = getDefaultAllocator();
  auto& second = getDefaultAllocator();
  EXPECT_EQ(&first, &second);
  EXPECT_EQ(first.resource(), second.resource());
}

TEST(ParserDefaultMemoryResourceTest, SetAfterCaptureFatals) {
  // Ensure the default allocator has been materialized (latches captured=true).
  (void)getDefaultAllocator().resource();
  // EXPECT_DEATH forks a child that inherits captured=true, so installing a
  // resource now is a too-late programming error and must FATAL.
  EXPECT_DEATH(
      setDefaultParserMemoryResource(std::pmr::get_default_resource()),
      "already materialized");
}

#endif // FOLLY_HAS_MEMORY_RESOURCE
