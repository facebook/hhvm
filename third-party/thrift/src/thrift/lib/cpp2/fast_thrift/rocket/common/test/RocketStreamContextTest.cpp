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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>

using namespace apache::thrift::fast_thrift::rocket;
using apache::thrift::fast_thrift::frame::FrameType;

TEST(RocketStreamContextTest, ContextDefaultsAreEmpty) {
  RocketStreamContext ctx;
  EXPECT_EQ(ctx.streamType, FrameType::RESERVED);
  EXPECT_EQ(ctx.credits, 0u);
}

// A freshly-constructed RocketStreamContexts wraps an empty stream map.
TEST(RocketStreamContextTest, ContextsStartEmpty) {
  RocketStreamContexts contexts;
  EXPECT_EQ(contexts.streams.size(), 0u);
}

// The one "connection" test: the RocketStreamContexts map stores and returns
// both fields of a RocketStreamContext. The map's own mechanics are covered by
// DirectStreamMapTest.
TEST(RocketStreamContextTest, MapStoresAndReturnsContextFields) {
  RocketStreamContexts contexts;
  contexts.streams.emplace(
      2u,
      RocketStreamContext{
          .streamType = FrameType::REQUEST_STREAM, .credits = 5});

  auto it = contexts.streams.find(2u);
  ASSERT_NE(it, contexts.streams.end());
  EXPECT_EQ(it->second.streamType, FrameType::REQUEST_STREAM);
  EXPECT_EQ(it->second.credits, 5u);
}
