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

#include <memory>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftRequestContext.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

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

// Request-response message carrying real metadata, as the transport adapter
// would hand it over.
ThriftServerRequestMessage makeRequestWithMetadata(
    uint32_t streamId,
    std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata) {
  ThriftServerRequestMessage req;
  req.streamId = streamId;
  req.payload = ThriftServerInboundPayloadVariant{ThriftRequestResponsePayload{
      .data = folly::IOBuf::copyBuffer("body"),
      .metadata = std::move(metadata)}};
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

TEST(
    ThriftServerRequestContextHandlerTest, StampsMethodNameOntoRequestContext) {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->name() = "Service.method";

  ThriftServerRequestContextHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(
              makeRequestWithMetadata(/*streamId=*/7, std::move(metadata)))),
      Result::Success);

  ASSERT_EQ(ctx.forwarded.size(), 1);
  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  ASSERT_NE(forwarded.requestContext, nullptr);
  EXPECT_EQ(forwarded.requestContext->getMethodName(), "Service.method");
}

// The name is copied, not moved out: the tail adapter dispatches on it, so
// emptying the metadata here would break routing.
TEST(ThriftServerRequestContextHandlerTest, LeavesMethodNameOnTheMetadata) {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->name() = "Service.method";

  ThriftServerRequestContextHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(
              makeRequestWithMetadata(/*streamId=*/7, std::move(metadata)))),
      Result::Success);

  ASSERT_EQ(ctx.forwarded.size(), 1);
  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  const auto* stamped = forwarded.payload.getRequestRpcMetadata();
  ASSERT_NE(stamped, nullptr);
  ASSERT_TRUE(stamped->name().has_value());
  EXPECT_EQ(stamped->name()->view(), "Service.method");
}

// A request whose metadata carries no name still gets a context; the name is
// simply empty.
TEST(
    ThriftServerRequestContextHandlerTest, MethodNameEmptyWhenMetadataHasNone) {
  ThriftServerRequestContextHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(
          ctx,
          erase_and_box(makeRequestWithMetadata(
              /*streamId=*/7,
              std::make_unique<apache::thrift::RequestRpcMetadata>()))),
      Result::Success);

  ASSERT_EQ(ctx.forwarded.size(), 1);
  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  ASSERT_NE(forwarded.requestContext, nullptr);
  EXPECT_TRUE(forwarded.requestContext->getMethodName().empty());
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
