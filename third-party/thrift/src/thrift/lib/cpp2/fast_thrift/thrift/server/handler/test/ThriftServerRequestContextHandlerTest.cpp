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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerRequestContextHandler.h>

#include <set>
#include <vector>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftRequestContext.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

using channel_pipeline::erase_and_box;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;

class FakeContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    forwarded.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    written.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception = std::move(e);
  }

  std::vector<TypeErasedBox> forwarded;
  std::vector<TypeErasedBox> written;
  folly::exception_wrapper exception;
};

ThriftServerRequestMessage makeRequest(uint32_t streamId = 1) {
  ThriftServerRequestMessage req;
  req.streamId = streamId;
  return req;
}

} // namespace

TEST(
    ThriftServerRequestContextHandlerTest,
    StampsDefaultRequestContextOnInboundMessage) {
  ThriftServerRequestContextHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(ctx, erase_and_box(makeRequest(/*sid=*/42))),
      Result::Success);

  ASSERT_EQ(ctx.forwarded.size(), 1);
  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  EXPECT_EQ(forwarded.streamId, 42);
  ASSERT_NE(forwarded.requestContext, nullptr);
  // Default-constructed: no conn context yet — that's a downstream handler's
  // job.
  EXPECT_EQ(forwarded.requestContext->getConnectionContext(), nullptr);
}

TEST(ThriftServerRequestContextHandlerTest, EachRequestGetsItsOwnContext) {
  ThriftServerRequestContextHandler<FakeContext> handler;
  FakeContext ctx;

  for (uint32_t sid = 1; sid <= 3; ++sid) {
    EXPECT_EQ(
        handler.onRead(ctx, erase_and_box(makeRequest(sid))), Result::Success);
  }

  std::set<ThriftRequestContext*> distinct;
  for (auto& box : ctx.forwarded) {
    auto& m = box.get<ThriftServerRequestMessage>();
    ASSERT_NE(m.requestContext, nullptr);
    distinct.insert(m.requestContext.get());
  }
  EXPECT_EQ(distinct.size(), 3) << "each request must get its own context";
}

TEST(ThriftServerRequestContextHandlerTest, ForwardsExceptions) {
  ThriftServerRequestContextHandler<FakeContext> handler;
  FakeContext ctx;

  handler.onException(
      ctx, folly::make_exception_wrapper<std::runtime_error>("boom"));

  ASSERT_TRUE(ctx.exception);
  EXPECT_NE(ctx.exception.what().toStdString().find("boom"), std::string::npos);
}

} // namespace apache::thrift::fast_thrift::thrift
